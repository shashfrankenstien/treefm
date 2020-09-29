#include <stdlib.h> /*malloc, realloc, free,...*/
#include <curses.h>
#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"


int print_dirlist_text(tfile* f, WINDOW* prv, int rows, int cols)
{
	if (!f->isdir) return -1;


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


void prv_show_preview(tdirlist* d, tree_app* app)
{
	werase(app->preview);

	int maxc = app->preview_props.cols - (2*app->preview_props.padc);
	int maxr = app->preview_props.rows;

	tfile* curfile = get_cur_tfile(d);

	char* prv_content = malloc(maxc*maxr * sizeof(char));

	if (curfile->isdir) {
		print_dirlist_text(curfile, app->preview, maxr, maxc);
	}

	free(prv_content);
}


