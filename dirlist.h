#include <linux/limits.h>


typedef struct tfile {
	char name[256];
	unsigned char type;
	unsigned short len;
	char fullpath[PATH_MAX];
	struct stat st;
	bool isdir;
	bool isexe;
	int _color_pair;
} tfile;


typedef struct tdirlist {
	char cwd[PATH_MAX];
	tfile* files;
	int files_count;
	int curs_pos;
} tdirlist;



// function signatures
tdirlist* listdir(const char*);
void free_tdirlist(tdirlist*);

tfile* get_tfile(tdirlist*, int);
short get_tfile_colorpair(tdirlist*, int);
