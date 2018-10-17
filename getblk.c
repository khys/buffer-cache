#include <stdio.h>
#include "buf.h"

struct buf_header *getblk(int blkno)
{
	struct buf_header *p;
	
	while (1) {
		if ((p = search_hash(blkno)) != NULL) {
			if (search_freelist(blkno) == NULL) {
				printf("scenario 5\n");
				// sleep();
				printf("Process goes to sleep\n");
				p->stat |= STAT_WAITED;
				return NULL;
			}
			printf("scenario 1\n");
			p->stat |= STAT_LOCKED;
			remove_from_freelist(p);
			return p;
		} else {
			if (freelist.free_fp == &freelist) {
				printf("scenario 4\n");
				// sleep();
				printf("Process goes to sleep\n");
				return NULL;
			}
			remove_from_freelist(p = freelist.free_fp);
			if ((p->stat & STAT_DWR) != 0x00000000) {
				printf("scenario 3\n");
				p->stat |= STAT_LOCKED;
				p->stat &= ~STAT_DWR;
				p->stat |= STAT_KRDWR;
				p->stat |= STAT_OLD;
				continue;
			}
			printf("scenario 2\n");
			remove_from_hash(p);
			p->stat |= STAT_LOCKED;
			p->blkno = blkno;
			p->stat &= ~STAT_VALID;
			insert_tail(&hash_head[hash(blkno)], p);
			return p;
		}
	}
}
