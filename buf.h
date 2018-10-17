#define NHASH 4
#define NBUF  12

#define BUFLEN 256
#define NARGS  256

#define STAT_LOCKED 0x00000001
#define STAT_VALID  0x00000002
#define STAT_DWR    0x00000004
#define STAT_KRDWR  0x00000008
#define STAT_WAITED 0x00000010
#define STAT_OLD    0x00000020

struct buf_header {
	int blkno;
	struct buf_header *hash_fp;
	struct buf_header *hash_bp;
	unsigned int stat;
	struct buf_header *free_fp;
	struct buf_header *free_bp;
	char *cache_data;
};

void getargs(int *, char *[], char *);

void help_proc(int, char *[]);
void init_proc(int, char *[]);
void buf_proc(int, char *[]);
void hash_proc(int, char *[]);
void free_proc(int, char *[]);
void getblk_proc(int, char *[]);
void brelse_proc(int, char *[]);
void set_proc(int, char *[]);
void reset_proc(int, char *[]);
void quit_proc(int, char *[]);

int hash(int blkno);

struct buf_header *search_hash(int blkno);
void insert_head(struct buf_header *h, struct buf_header *p);
void insert_tail(struct buf_header *h, struct buf_header *p);
void remove_from_hash(struct buf_header *p);

struct buf_header *search_freelist(int blkno);
void insert_head_of_freelist(struct buf_header *p);
void insert_tail_of_freelist(struct buf_header *p);
void remove_from_freelist(struct buf_header *p);

struct buf_header *getblk(int blkno);
void brelse(struct buf_header *buffer);
void getargs(int *ac, char *av[], char *p);
void print_buf(int bufno);
void print_hash(int h);

extern struct buf_header hash_head[NHASH];
extern struct buf_header freelist;
extern struct buf_header buf[NBUF];
