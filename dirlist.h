
typedef struct tfile {
	char name[256];
	unsigned char type;
	unsigned short len;
	const char* fullpath;
	struct stat* st;
	bool isdir;
	bool isexe;
	int _color_pair;
} tfile;


typedef struct tdirlist {
	tfile* files;
	int dirs_count;
	int files_count;
	int curs_pos;
} tdirlist;



// function signatures
tdirlist* listdir(const char*);
void free_tfile(tfile*);
void free_tdirlist(tdirlist*);
void sortnames(tfile*, const int);
tfile* sort(tfile*, int, tfile*, int);
tfile* get_tfile(tdirlist*, int);
short get_tfile_colorpair(tdirlist*, int);
