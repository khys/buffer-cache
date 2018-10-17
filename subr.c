#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "buf.h"
	
int hash(int blkno)
{
	return blkno % NHASH;
}

struct buf_header *search_hash(int blkno)
{
	int h;
	struct buf_header *p;
	
	h = hash(blkno);
	for (p = hash_head[h].hash_fp; p != &hash_head[h]; p = p->hash_fp) {
		if (p->blkno == blkno) {
			return p;
		}
	}
	return NULL;
}

void insert_head(struct buf_header *h, struct buf_header *p)
{
	p->hash_bp = h;
	p->hash_fp = h->hash_fp;
	h->hash_fp->hash_bp = p;
	h->hash_fp = p;
}

void insert_tail(struct buf_header *h, struct buf_header *p)
{
	p->hash_fp = h;
	p->hash_bp = h->hash_bp;
	h->hash_bp->hash_fp = p;
	h->hash_bp = p;
}

void remove_from_hash(struct buf_header *p)
{
	if (p->hash_fp != NULL && p->hash_bp != NULL) {
		p->hash_bp->hash_fp = p->hash_fp;
		p->hash_fp->hash_bp = p->hash_bp;
		p->hash_fp = p->hash_bp = NULL;
	}
}

struct buf_header *search_freelist(int blkno)
{
	struct buf_header *p;
	
	for (p = freelist.free_fp; p != &freelist; p = p->free_fp) {
		if (p->blkno == blkno) {
			return p;
		}
	}
	return NULL;
}

void insert_head_of_freelist(struct buf_header *p)
{
	p->free_bp = &freelist;
	p->free_fp = freelist.free_fp;
	freelist.free_fp->free_bp = p;
	freelist.free_fp = p;
}

void insert_tail_of_freelist(struct buf_header *p)
{
	p->free_fp = &freelist;
	p->free_bp = freelist.free_bp;
	freelist.free_bp->free_fp = p;
	freelist.free_bp = p;
}

void remove_from_freelist(struct buf_header *p)
{
	p->free_bp->free_fp = p->free_fp;
	p->free_fp->free_bp = p->free_bp;
	p->free_fp = p->free_bp = NULL;
}

void brelse(struct buf_header *buffer)
{
	if (freelist.free_fp == &freelist) {
		// wakeup();
		printf("Wakeup processes waiting for any buffer\n");
	} else if ((buffer->stat & STAT_WAITED) != 0x00000000) {
		// wakeup();
		printf("Wakeup processes waiting for buffer of blkno %d\n",
			   buffer->blkno);
		buffer->stat &= ~STAT_WAITED;
	}
	// raise_cpu_level();
	if ((buffer->stat & STAT_VALID) != 0x00000000 &&
		(buffer->stat & STAT_OLD) == 0x00000000) {
		insert_tail_of_freelist(buffer);
		printf("buf(blkno %d) was inserted at tail of free list\n",
			   buffer->blkno);
	} else {
		insert_head_of_freelist(buffer);
		printf("buf(blkno %d) was inserted at top of free list\n",
			   buffer->blkno);
		buffer->stat &= ~STAT_OLD;
	}
	// lower_cpu_level();
	
	// unlock(buffer);
	buffer->stat &= ~STAT_LOCKED;
}

void getargs(int *ac, char *av[], char *p)
{
	*ac = 0;

	for (;;) {
		while (isblank(*p)) {
			p++;
		}
		if (*p == '\0') {
			return;
		}
		av[(*ac)++] = p;
		while (*p && !isblank(*p)) {
			p++;
		}
		if (*p == '\0') {
			return;
		}
		*p++ = '\0';
	}
}

void print_buf(int bufno)
{
	printf("[%2d: %2d ", bufno, buf[bufno].blkno);
	if ((buf[bufno].stat & STAT_OLD) == 0x00000000) {
		printf("-");
	} else {
		printf("O");
	}
	if ((buf[bufno].stat & STAT_WAITED) == 0x00000000) {
		printf("-");
	} else {
		printf("W");
	}
	if ((buf[bufno].stat & STAT_KRDWR) == 0x00000000) {
		printf("-");
	} else {
		printf("K");
	}
	if ((buf[bufno].stat & STAT_DWR) == 0x00000000) {
		printf("-");
	} else {
		printf("D");
	}
	if ((buf[bufno].stat & STAT_VALID) == 0x00000000) {
		printf("-");
	} else {
		printf("V");
	}
	if ((buf[bufno].stat & STAT_LOCKED) == 0x00000000) {
		printf("-");
	} else {
		printf("L");
	}
	printf("]");
}

void print_hash(int h)
{	
	struct buf_header *p;

	printf("%2d: ", h);
	for (p = hash_head[h].hash_fp; p != &hash_head[h]; p = p->hash_fp) {
		print_buf(p - buf);
		printf(" ");
	}
	printf("\n");
}

void help_proc(int argc, char *argv[])
{
	printf("help:                    show help\n");
	printf("init:                    initialize hash list and free list\n");
	printf("buf [n]:                 show buffer headers\n");
	printf("hash [n]:                show hash lists\n");
	printf("free:                    show free list\n");
	printf("getblk n:                get block 'n'\n");
	printf("brelse n:                release block 'n'\n");
	printf("set n stat [stat ...]:   set status 'stat' in block 'n'\n");
	printf("reset n stat [stat ...]: reset status 'stat' in block 'n'\n");
	printf("quit:                    quit\n");
}

void init_proc(int argc, char *argv[])
{
	int i;
	
	for (i = 0; i < NHASH; i++) {
		hash_head[i].hash_fp = hash_head[i].hash_bp = &hash_head[i];
	}
	freelist.free_fp = freelist.free_bp = &freelist;
	for (i = 0; i < NBUF; i++) {
		buf[i].hash_fp = buf[i].hash_bp = NULL;
		buf[i].free_fp = buf[i].hash_bp = NULL;
		buf[i].stat &= 0x00000000;
		insert_tail_of_freelist(&buf[i]);
	}
	
	getblk(28); getblk(4); getblk(64); getblk(17);
	getblk(5); getblk(97); getblk(98); getblk(50);
	getblk(10); getblk(3); getblk(35); getblk(99);
	
	for (i = 0; i < NBUF; i++) {
		buf[i].stat |= STAT_VALID;
	}
	
	brelse(search_hash(3)); brelse(search_hash(5));
	brelse(search_hash(4)); brelse(search_hash(28));
	brelse(search_hash(97)); brelse(search_hash(10));
}

void buf_proc(int argc, char *argv[])
{
	int i, bufno;
	char *e;
	
	if (argc == 1) {
		for (i = 0; i < NBUF; i++) {
			print_buf(i);
			printf("\n");
		}
	} else {
		for (i = 1; i < argc; i++) {
			bufno = (int)strtol(argv[i], &e, 10);
			if (*e != '\0') {
				printf("Usage: buf [n ...]\n");
				return;
			} else if (bufno < 0 || bufno >= NBUF) {
				printf("illegal buffer no: %d\n", bufno);
				return;
			} else {
				print_buf(bufno);
				printf("\n");
			}
		}
	}
}

void hash_proc(int argc, char *argv[])
{
	int i, h;
	char *e;
	
	if (argc == 1) {
		for (i = 0; i < NHASH; i++) {
			print_hash(i);
		}
	} else {
		for (i = 1; i < argc; i++) {
			h = (int)strtol(argv[i], &e, 10);
			if (*e != '\0') {
				printf("Usage: hash [n ...]\n");
				return;
			} else if (h < 0 || h >= NHASH) {
				printf("illegal hash no: %d\n", h);
				return;
			} else {
				print_hash(h);
			}
		}
	}
}

void free_proc(int argc, char *argv[])
{
	struct buf_header *p;
	
	for (p = freelist.free_fp; p != &freelist; p = p->free_fp) {
	    print_buf(p - buf);
		printf(" ");
	}
	printf("\n");
}

void getblk_proc(int argc, char *argv[])
{
	int blkno;
	char *e;

	blkno = (int)strtol(argv[1], &e, 10);
	if (*e == '\0') {
		getblk(blkno);
	} else {
		printf("Usage: getblk n\n");
		return;
	}
}

void brelse_proc(int argc, char *argv[])
{
	int blkno;
	char *e;
	
	blkno = (int)strtol(argv[1], &e, 10);
	if (*e == '\0') {
		brelse(search_hash(blkno));
	} else {
		printf("Usage: brelse n\n");
		return;
	}
}

void set_proc(int argc, char *argv[])
{
	int i, blkno;
	char *e;

	blkno = (int)strtol(argv[1], &e, 10);
	if (*e != '\0' || argc < 3) {
		printf("Usage: set n stat [stat ...]\n");
		return;
	} else if (search_hash(blkno) == NULL) {
		printf("illegal block no: %d\n", blkno);
		return;
	} else {
		for (i = 2; i < argc; i++) {
			if (!strcmp(argv[i], "L")) {
				search_hash(blkno)->stat |= STAT_LOCKED;
			} else if (!strcmp(argv[i], "V")) {
				search_hash(blkno)->stat |= STAT_VALID;
			} else if (!strcmp(argv[i], "D")) {
				search_hash(blkno)->stat |= STAT_DWR;
			} else if (!strcmp(argv[i], "K")) {
				search_hash(blkno)->stat |= STAT_KRDWR;
			} else if (!strcmp(argv[i], "W")) {
				search_hash(blkno)->stat |= STAT_WAITED;
			} else if (!strcmp(argv[i], "O")) {
				search_hash(blkno)->stat |= STAT_OLD;
			} else {
				printf("Usage: set n stat [stat ...]\n");
				return;
			}
		}
	}
}

void reset_proc(int argc, char *argv[])
{
	int i, blkno;
	char *e;

	blkno = (int)strtol(argv[1], &e, 10);
	if (*e != '\0' || argc < 3) {
		printf("Usage: reset n stat [stat ...]\n");
		return;
	} else if (search_hash(blkno) == NULL) {
		printf("illegal block no: %d\n", blkno);
		return;
	} else {
		if (!strcmp(argv[2], "L")) {
			search_hash(blkno)->stat &= ~STAT_LOCKED;
		} else if (!strcmp(argv[2], "V")) {
			search_hash(blkno)->stat &= ~STAT_VALID;
		} else if (!strcmp(argv[2], "D")) {
			search_hash(blkno)->stat &= ~STAT_DWR;
		} else if (!strcmp(argv[2], "K")) {
			search_hash(blkno)->stat &= ~STAT_KRDWR;
		} else if (!strcmp(argv[2], "W")) {
			search_hash(blkno)->stat &= ~STAT_WAITED;
		} else if (!strcmp(argv[2], "O")) {
			search_hash(blkno)->stat &= ~STAT_OLD;
		} else {
			printf("Usage: reset n stat [stat ...]\n");
			return;
		}
	}
}

void quit_proc(int argc, char *argv[]) {
	exit(0);
}
