#include <stdlib.h> /*malloc, realloc, free, true, false*/
#include <string.h> /*strcmp,strcpy,..*/
#include <dirent.h>
#include <sys/stat.h>

#include "utils.h" /*macros*/
#include "dirlist.h" /*declarations*/


// function implementations

void free_tdirlist(tdirlist* d)
{
	free(d->files);
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


void sort_copy(tfile* d, const int len, tfile** dst)
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
	*dst = (tfile*) malloc(sizeof(tfile)*len);

	for (int a=0; a<dircount; a++) {
		(*dst)[dstcounter++] = *dirs[a];
		// printf("dir %s\n", dirs[a]->name);
	}
	for (int a=0; a<filecount; a++) {
		(*dst)[dstcounter++] = *files[a];
		// printf("file %s\n", files[a]->name);
	}
}


void joinpath(const char* dirname, const char* basename, char* buf)
{
	strcpy(buf, dirname);
	strcat(buf, "/");
	strcat(buf, basename);
}


bool is_executable(unsigned int st_mode)
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
	tfile* files = (tfile*) malloc(sizeof(tfile)*files_cap);
	tdirlist* out = (tdirlist*)malloc(sizeof(tdirlist));
	strcpy(out->cwd, cpath);

	d = opendir(cpath);
	if (d)
	{
		int files_count = 0;
		while ((dir = readdir(d))!=NULL)
		{
			if (files_count==files_cap)
			{// limit reached, reallocate array
				files_cap*=2;
				files = realloc(files, sizeof(tfile)*files_cap);
			}

			strcpy(files[files_count].name, dir->d_name);
			files[files_count].type = dir->d_type;
			files[files_count].len = dir->d_reclen;

			char combined[strlen(cpath) + strlen(dir->d_name) + 2];
			joinpath(cpath, dir->d_name, combined);
			realpath(combined, files[files_count].fullpath);

			stat(files[files_count].fullpath, &files[files_count].st);

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

		sort_copy(files, files_count, &out->files);
		out->files_count = files_count;
		closedir(d);
	}
	free(files);
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


