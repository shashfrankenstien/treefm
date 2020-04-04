#include <stdlib.h> /*malloc, realloc, free, true, false*/
#include <string.h> /*strcmp,strcpy,..*/
#include <dirent.h>
#include <limits.h> /*PATH_MAX*/
#include <sys/stat.h>

#include "utils.h" /*macros*/
#include "dirlist.h" /*declarations*/


// function implementations
void free_tfile(tfile* f)
{
	free((char*)f->fullpath);
	free(f->st);
}

void free_tdirlist(tdirlist* d)
{
	for (int i=0; i < d->dirs_count+d->files_count; i++)
		free_tfile(&d->files[i]);
	free(d->files);
	free(d);
}


void sortnames(tfile* d, const int len) //bubble sort
{
	tfile tmp;
	bool swapped = false; //setup swapped flag
	for (int i=0; i<len-1; i++)
	{
		int comp = strcmp(d[i].name, d[i+1].name);
		if (comp > 0)
		{//swap
			tmp = d[i+1];
			d[i+1] = d[i];
			d[i] = tmp;
			swapped = true;//redo while loop if swapped
		}
	}
	if (swapped)
		sortnames(d, len);//recurse!
}


tfile* sort(tfile* dirs, int dirs_count, tfile* files, int files_count)
{
	sortnames(dirs, dirs_count);
	sortnames(files, files_count);
	tfile* sorted = (tfile*)malloc(sizeof(tfile) * (dirs_count+files_count));
	for (int i=0; i < dirs_count; i++)
		sorted[i] = dirs[i];
	for (int i=0; i < files_count; i++)
		sorted[i+dirs_count] = files[i];
	return sorted;
}


void joinpath(const char* dirname, const char* basename, char* buf)
{
	strcpy(buf, dirname);
	strcat(buf, "/");
	strcat(buf, basename);
}


tdirlist* listdir(const char* cpath)
{
	DIR *d;
	struct dirent *dir;
	int dirs_cap = 10;
	int files_cap = 10;
	tfile* dirs = (tfile*) malloc(sizeof(tfile)*dirs_cap);
	tfile* files = (tfile*) malloc(sizeof(tfile)*files_cap);
	tdirlist* out = (tdirlist*)malloc(sizeof(tdirlist));
	strcpy(out->cwd, cpath);
	//out->cwd = (char*)cpath;

	d = opendir(cpath);
	if (d)
	{
		int dirs_count = 0;
		int files_count = 0;
		while ((dir = readdir(d))!=NULL)
		{
			char combined[strlen(cpath) + strlen(dir->d_name) + 2];
			joinpath(cpath, dir->d_name, combined);
			const char* fullpath = realpath(combined, NULL);
			struct stat* st = (struct stat*)malloc(sizeof(struct stat));
			stat(fullpath, st);

			if (dir->d_type==DT_DIR)
			{// if directory
				if (dirs_count==dirs_cap)
				{// limit reached, reallocate array
					dirs_cap *= 2;
					dirs = realloc(dirs, sizeof(tfile)*dirs_cap);
				}
				strcpy(dirs[dirs_count].name, dir->d_name);
				dirs[dirs_count].type = dir->d_type;
				dirs[dirs_count].len = dir->d_reclen;
				dirs[dirs_count].st = st;
				dirs[dirs_count].fullpath = fullpath;
				dirs[dirs_count].isdir = true;
				dirs[dirs_count].isexe = false;
				dirs[dirs_count]._color_pair = DIR_COLOR;
				dirs_count++;
			}
			else
			{// if not directory
				if (files_count==files_cap)
				{// limit reached, reallocate array
					files_cap*=2;
					files = realloc(files, sizeof(tfile)*files_cap);
				}
				strcpy(files[files_count].name, dir->d_name);
				files[files_count].type = dir->d_type;
				files[files_count].len = dir->d_reclen;
				files[files_count].st = st;
				files[files_count].fullpath = fullpath;
				files[files_count].isdir = false;
				files[files_count].isexe = (st->st_mode & S_IXUSR || st->st_mode & S_IXGRP || st->st_mode & S_IXOTH);
				if (files[files_count].isexe)
					files[files_count]._color_pair = EXE_COLOR;
				else
					files[files_count]._color_pair = NORM_COLOR;
				files_count++;
			}
		}

		out->files = sort(dirs, dirs_count, files, files_count);
		out->dirs_count = dirs_count;
		out->files_count = files_count;
		closedir(d);
	}
	return out;
}


tfile* get_tfile(tdirlist* d, int idx)
{
	int maxidx = d->dirs_count + d->files_count - 1;
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


