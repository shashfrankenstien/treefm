// #include <stdlib.h> /*malloc, realloc, free,...*/
#include <curses.h>
#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"


static int print_dirlist_text(tfile* f, WINDOW* prv, int rows, int cols)
{
	if (!f->isdir) return -1; // redundant check

	tdirlist* d = listdir(f->fullpath);

	for (int i=0; i<d->files_count && i<rows; i++) {
		wmove(prv, i, 1);

		if (d->files_count-1==i) // last element reached
			waddch(prv, ACS_LLCORNER);
		else
			waddch(prv, ACS_LTEE);
		waddch(prv, ACS_HLINE);
		waddch(prv, ACS_HLINE);

		const char* namefmt = (d->files[i].isdir && DIRNAME_WITH_SLASH) ? " %s/" : " %s";
		wprintw(prv, namefmt, d->files[i].name);
	}

	free_tdirlist(d);
	return 0;
}


static int is_binary_file(FILE* fp)
{
	/* check if binary file by seeing if there are any
	*	non-ascii characters in the first n bytes defined by test_len
	*/
	int test_len = 100;
	int c, i = 0;
	int is_bin = 0;

	while ((c = getc(fp)) != EOF && i <= test_len) {
		if (c < 32 && c != '\n' && c != '\t' && c != '\r') {
			is_bin = 1;
			break;
		}
		i++;
	}
	fseek(fp, 0, SEEK_SET);
	return is_bin;
}


static int print_file_text(tfile* f, WINDOW* prv, int rows, int cols)
{
	if (f->isdir) return -1; // redundant check

	FILE* fp = fopen(f->fullpath, "r");
	if (fp == NULL)
		return -1;

	if (is_binary_file(fp)==1) // don't preview binary files
		return -1;

	int c, i = 0;
	while ((c = getc(fp)) != EOF && i <= rows*cols) {
		waddch(prv, c);
		i++;
	}

	fclose(fp);
	return 0;
}


void prv_show_preview(tdirlist* d, tree_app* app)
{
	werase(app->preview);

	int maxc = app->preview_props.cols - (2*app->preview_props.padc);
	int maxr = app->preview_props.rows;

	tfile* curfile = get_cur_tfile(d);

	if (curfile->isdir) {
		print_dirlist_text(curfile, app->preview, maxr, maxc);
	}
	else {
		print_file_text(curfile, app->preview, maxr, maxc);
	}
}


