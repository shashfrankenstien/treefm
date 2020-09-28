#include <stdlib.h> /*malloc, realloc, free,...*/
#include <curses.h>
#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"
#include "preview.h"


void prv_show_preview(tdirlist* d, tree_app* app)
{
	werase(app->preview);

	int maxc = app->preview_props.cols - (2*app->preview_props.padc);
	int maxr = app->preview_props.rows;

	tfile* curfile = get_cur_tfile(d);

	char* prv_content = malloc(maxc*maxr * sizeof(char));

	if (curfile->isdir) {
		if(get_dirlist_text(curfile, prv_content, maxc*maxr)==0)
			mvwprintw(app->preview, 0, 0, "%s\n", prv_content);
	}

	free(prv_content);
}


int get_dirlist_text(tfile* f, char* content, int size_limit)
{
	if (!f->isdir) return -1;
	tdirlist* d = listdir(f->fullpath);

	int start_len = 0;
	for (int i=0; i<d->files_count && start_len<size_limit; i++) {
		int l = strlen(d->files[i].name)+5;
		start_len += l;
		if (start_len > size_limit)
			l -= (start_len-size_limit);

		const char* namefmt;
		if (d->files[i].isdir && DIRNAME_WITH_SLASH) {
			namefmt = "|--%s/\n";
			l += 1;
		}
		else {
			namefmt = "|--%s\n";
		}
		snprintf(content, l, namefmt, d->files[i].name);
		content += l-1;
	}

	free_tdirlist(d);
	return 0;
}
