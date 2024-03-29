#include <stdio.h>

#include <stdlib.h> /*malloc, realloc, free,...*/
#include <string.h> /*strcmp,strcpy,..*/
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "utils.h" /*macros*/
#include "dirlist.h" /*declarations*/

#include "config.h"


#ifdef __WINDOWSOS__
char* realpath(const char* path, char resolved[PATH_MAX])
{
	GetFullPathName(path,_MAX_PATH,resolved,NULL);
	return resolved;
}
#endif


// function implementations
tdirlist* new_tdirlist()
{
	tdirlist* l = (tdirlist*)malloc(sizeof(tdirlist));
	//setup default values
	l->files = NULL;
	l->files_count = 0;
	l->curs_pos = 0;
	l->err = 0;
	return l;
}

void free_tdirlist(tdirlist* d)
{
	if (d->files) free(d->files);
	free(d);
}


/*bubble sort
sorts array of tfile pointers alphabetically
*/
void sortname_ptrs(tfile** d, const int len)
{
	bool swapped = false; //setup swapped flag
	for (int i=0; i<len-1; i++)
	{
		int comp = strcmp(d[i]->name, d[i+1]->name);
		if (comp > 0)
		{//swap
			tfile* tmp = d[i+1];
			d[i+1] = d[i];
			d[i] = tmp;
			swapped = true;//redo while loop if swapped
		}
	}
	if (swapped)
		sortname_ptrs(d, len);//recurse!
}


void sort_copy(tfile* d, const int len, tfile* dst)
{
	tfile* dirs[len];
	tfile* files[len];
	int dircount = 0, filecount = 0;
	for (int i=0; i<len; i++) {
		if (d[i].isdir) {
			dirs[dircount++] = &d[i];
		} else {
			files[filecount++] = &d[i];
		}
	}
	sortname_ptrs(&dirs[0], dircount);
	sortname_ptrs(&files[0], filecount);

	int dstcounter = 0;

	for (int a=0; a<dircount; a++) {
		dst[dstcounter++] = *dirs[a];
		// printf("dir %s\n", dirs[a]->name);
	}
	for (int a=0; a<filecount; a++) {
		dst[dstcounter++] = *files[a];
		// printf("file %s\n", files[a]->name);
	}
}


void joinpath(const char* dirname, const char* basename, char* buf)
{
	strcpy(buf, dirname);
	strcat(buf, "/");
	strcat(buf, basename);
}


void trim_home_inplace(char* path)
{
	char* home_path = getenv(HOMEPATH_ENV_VAR);
	if (home_path!=NULL) {
		char* match = strstr(path, home_path);
		if (match!=NULL) {
			size_t home_len = strlen(home_path);
			snprintf(path, strlen(path)-home_len+2, "~%s%c", match+home_len, '\0');
		}
	}
}


bool is_executable(mode_t st_mode)
{
	return S_ISREG(st_mode) && (
		st_mode & S_IXUSR
		|| st_mode & S_IXGRP
		|| st_mode & S_IXOTH
	);
}


tdirlist* listdir(const char* cpath)
{
	DIR *d;
	struct dirent *dir;
	int files_cap = 10;
	tdirlist* out = new_tdirlist();
	realpath(cpath, out->cwd);

	if (TRIM_HOME_DIR_PATH) trim_home_inplace(out->cwd);

	d = opendir(cpath);
	if (d)
	{
		tfile* files = (tfile*) malloc(sizeof(tfile)*files_cap);
		int files_count = 0;
		while ((dir = readdir(d))!=NULL)
		{
			if (strcmp(dir->d_name, ".")==0) // ignore current directory "."
				continue;

			if (files_count==files_cap)
			{// limit reached, reallocate array
				files_cap*=2;
				files = realloc(files, sizeof(tfile)*files_cap);
			}

			strcpy(files[files_count].name, dir->d_name);
			files[files_count].len = dir->d_reclen;

			char combined[strlen(cpath) + strlen(dir->d_name) + 2];
			joinpath(cpath, dir->d_name, combined);
			realpath(combined, files[files_count].fullpath);

			stat(files[files_count].fullpath, &files[files_count].st);

			files[files_count].type = IFTODT(files[files_count].st.st_mode);
			files[files_count].isdir = S_ISDIR(files[files_count].st.st_mode);
			files[files_count].isexe = is_executable(files[files_count].st.st_mode);

			if (files[files_count].isexe)
				files[files_count]._color_pair = EXE_COLOR;
			else if (files[files_count].isdir)
				files[files_count]._color_pair = DIR_COLOR;
			else
				files[files_count]._color_pair = NORM_COLOR;
			files_count++;
		}

		out->files = (tfile*) malloc(sizeof(tfile)*files_count);
		sort_copy(files, files_count, out->files);
		out->files_count = files_count;
		closedir(d);
		free(files);
	}
	else {
		out->err = errno;
	}
	return out;
}


tfile* get_tfile(tdirlist* d, int idx)
{
	int maxidx = d->files_count - 1;
	if (idx >= 0 && idx <= maxidx)
		return &d->files[idx];
	else
		return NULL;
}

short get_tfile_colorpair(tdirlist* d, int idx)
{
	tfile* f = get_tfile(d, idx);
	if (f)
		return f->_color_pair;
	else
		return 0;
}

tfile* get_cur_tfile(tdirlist* d)
{
	return get_tfile(d, d->curs_pos);
}

short get_cur_tfile_colorpair(tdirlist* d)
{
	return get_tfile_colorpair(d, d->curs_pos);
}
