#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "buf.h"

struct buf_header hash_head[NHASH];
struct buf_header freelist;
struct buf_header buf[NBUF];

struct command_table {
	char *cmd;
	void (*func)(int, char *[]);
} cmd_tbl[] = {
	{"help", help_proc},
	{"init", init_proc},
	{"buf", buf_proc},
	{"hash", hash_proc},
	{"free", free_proc},
	{"getblk", getblk_proc},
	{"brelse", brelse_proc},
	{"set", set_proc},
	{"reset", reset_proc},
	{"quit", quit_proc},
	{NULL, NULL}
};

int main()
{
	struct command_table *p;
   	char lbuf[BUFLEN], *argv[NARGS];
	int argc, i;

	for (i = 0; i < NHASH; i++) {
		hash_head[i].hash_fp = hash_head[i].hash_bp = &hash_head[i];
	}
	freelist.free_fp = freelist.free_bp = &freelist;
	
	for (;;) {
		fprintf(stderr, "$ ");
		if (fgets(lbuf, sizeof lbuf, stdin) == NULL) {
			putchar('\n');
			return 0;
		}
		lbuf[strlen(lbuf) - 1] = '\0';
		if (*lbuf == '\0') {
			continue;
		}

		getargs(&argc, argv, lbuf);

		for (p = cmd_tbl; p->cmd; p++) {
			if (!strcmp(argv[0], p->cmd)) {
				(*p->func)(argc, argv);
				break;
			}
		}
		if (p->cmd == NULL) {
			fprintf(stderr, "unknown command\n");
		}
	}
	
	return 0;
}
