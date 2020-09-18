#pragma once

#if defined(_WIN64) || defined(_WIN32)
	#ifndef __WINDOWSOS__
		#define __WINDOWSOS__
	#endif

	#include <windows.h>
	char* realpath(const char* file,char* path);
	#define lstat _stat
	#define stat  _stat

	#ifndef PATH_MAX
		#define PATH_MAX 1024
	#endif
#else
	#ifndef _POSIX_C_SOURCE
		#define _POSIX_C_SOURCE 1
	#endif
	#include <limits.h>
#endif

#define IFTODT(mode) (((mode) & 0170000) >> 12)



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
	int err;
} tdirlist;



// function signatures
tdirlist* listdir(const char*);
void free_tdirlist(tdirlist*);

tfile* get_tfile(tdirlist*, int);
short get_tfile_colorpair(tdirlist*, int);
