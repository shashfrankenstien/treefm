#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"


// function declarations
void _fmt_fsize(long int, char*);
void show_row(tfile*, WINDOW*, int, int, int);
void show_tdirlist(tdirlist*, tree_app*);

int kbd_events(tdirlist**, tree_app*);
void vnavigate(tdirlist*, tree_app*, e_vertical);
void hnavigate(tdirlist**, tree_app*, e_horizontal);



void _fmt_fsize(long int sz, char* szstr)
{
	float szf;
	size_t slen = strlen(szstr);
	if (sz < 1024)
		snprintf(szstr, slen, "%ld", sz);
	else if ((szf = sz/1024.) < 10.)
		snprintf(szstr, slen, "%.1fK", szf);
	else if ((szf = sz/1024.) < 1000)
		snprintf(szstr, slen, "%.0fK", szf);
	else if ((szf = sz/1024./1024.) < 10)
		snprintf(szstr, slen, "%.1fM", szf);
	else if ((szf = sz/1024./1024.) < 1000)
		snprintf(szstr, slen, "%.0fM", szf);
	else if ((szf = sz/1024./1024./1024.) < 1000)
		snprintf(szstr, slen, "%.0fG", szf);
	else
		snprintf(szstr, slen, "%.0fT", szf/1024./1024./1024./1024.);
}


void show_row(tfile* f, WINDOW* win, int curr, int curc, int maxc)
{
	if (f->_color_pair!=NORM_COLOR)
		wattron(win, A_BOLD | COLOR_PAIR(f->_color_pair));

	char sz[] = "00000";
	_fmt_fsize(f->st.st_size, sz);
	mvwprintw(win, curr, curc, "%s", f->name);
	mvwprintw(win, curr, maxc-strlen(sz), "%s", sz);

	if (f->_color_pair!=NORM_COLOR)
		wattroff(win, A_BOLD | COLOR_PAIR(f->_color_pair));
}


void show_tdirlist(tdirlist* d, tree_app* app)
{
	erase_app(app);

	int maxc = app->brw_props.cols - (2*app->brw_props.padc);
	int curc = app->brw_props.startc + app->brw_props.padc;

	int visible_rows = iMIN(app->brw_props.rows, d->files_count);
	for (int r=0; r < visible_rows; r++)
		show_row(&d->files[r], app->browser, r, curc, maxc);

	mvwchgat(app->browser, app->brw_props.curs_pos, 0,
		-1, A_REVERSE|A_BOLD,
		get_tfile_colorpair(d, d->curs_pos), NULL);

	if (app->brw_props.rows < d->files_count) {
		scrollok(app->browser, true); // allows browser (main) window to scroll
	}

	write_header(app, d->cwd, LEFT);
	// write_footer(app, d->cwd, LEFT);
	char count[10];
	snprintf(count, 10, "%d/%d", d->curs_pos+1, d->files_count);
	write_cmd(app, count, RIGHT);
}





void vnavigate(tdirlist* d, tree_app* app, e_vertical direction)
{
	// reset color highlight on current position
	int cp = get_tfile_colorpair(d, d->curs_pos);
	int gatp = A_NORMAL;

	if (cp!=NORM_COLOR)
		gatp |= A_BOLD;
	mvwchgat(app->browser, app->brw_props.curs_pos, 0, -1, gatp, cp, NULL); // reset color

	if (direction==DOWN) {
		if (app->brw_props.curs_pos + SCROLL_OFFSET + 1 >= app->brw_props.rows
			&& (d->files_count-1) - d->curs_pos > SCROLL_OFFSET) { // need to scroll down
			wscrl(app->browser, 1);
			// write a new row
			int edge_idx = d->curs_pos + SCROLL_OFFSET + 1;
			int maxc = app->brw_props.cols - (2*app->brw_props.padc);
			int curc = app->brw_props.startc + app->brw_props.padc;
			show_row(&d->files[edge_idx], app->browser, app->brw_props.rows-1, curc, maxc);

		} else {
			// not scrolling. move cursor down
			app->brw_props.curs_pos = iMIN(
				iMIN(app->brw_props.curs_pos + 1, (d->files_count - 1)), // files count limit for short lists
				app->brw_props.rows-1 // browser last row
			);
		}
		d->curs_pos = iMIN(d->curs_pos + 1, (d->files_count - 1)); // increment file pointer pos
	}
	else {
		if (app->brw_props.curs_pos - SCROLL_OFFSET <= 0
			&& d->curs_pos > app->brw_props.curs_pos) { // need to scroll up
			wscrl(app->browser, -1);
			// write a new row
			int edge_idx = d->curs_pos - app->brw_props.curs_pos - 1;
			int maxc = app->brw_props.cols - (2*app->brw_props.padc);
			int curc = app->brw_props.startc + app->brw_props.padc;
			show_row(&d->files[edge_idx], app->browser, 0, curc, maxc);

		} else {
			// not scrolling. move cursor up
			app->brw_props.curs_pos = iMAX(app->brw_props.curs_pos - 1, 0);
		}
		d->curs_pos = iMAX(d->curs_pos - 1, 0); // decrement file pointer pos
	}

	mvwchgat(app->browser, app->brw_props.curs_pos, 0,
		-1, A_REVERSE|A_BOLD,
		get_tfile_colorpair(d, d->curs_pos), NULL); // highlight new cursor position

	char count[10];
	snprintf(count, 10, "%d/%d", d->curs_pos+1, d->files_count);
	write_cmd(app, count, RIGHT);

}


void hnavigate(tdirlist** d, tree_app* app, e_horizontal direction)
{
	if (direction==RIGHT) {
		tfile curfile = (*d)->files[ (*d)->curs_pos ];
		if (curfile.isdir)
		{
			tdirlist* new_d = listdir((const char*)curfile.fullpath);
			write_header(app, strerror(new_d->err), RIGHT);
			if (new_d->err==0) {
				free_tdirlist(*d);
				*d = new_d;
				app->brw_props.curs_pos = 0; // reset cursor to top. TODO - parse path and move to directory
				show_tdirlist(*d, app);
			}
			else {
				free_tdirlist(new_d);
			}
		}
	}
}



int kbd_events(tdirlist** d, tree_app* app)
{
	int ch;
	while((ch = getch()) != KEY_F(1))
	{
		switch(ch)
		{
			case KEY_RESIZE:
			return RESIZE;
			break;

			case KEY_LEFT:
			case 'h':
			break;

			case KEY_RIGHT:
			case 'l':
			hnavigate(d, app, RIGHT);
			break;

			case KEY_UP:
			case 'k':
			vnavigate(*d, app, UP);
			break;

			case KEY_DOWN:
			case 'j':
			vnavigate(*d, app, DOWN);
			break;
		}
		char* c = (char*)&ch;
		write_cmd(app, c, LEFT);
		refresh_app(app);
	}
	return EXIT;
}



int main(int argc, char* argv[])
{
	const char* p;
	if (argc>1)
		p = argv[1];
	else
		p = ".";

	tdirlist* d = listdir(p);
	d->curs_pos = 0; //cursor position on top of the list

	tree_app app;
	create_app(&app);

	show_tdirlist(d, &app);
	refresh_app(&app);

	int action;
	bool exit = false;
	while (!exit)
	{
		action = kbd_events(&d, &app);
		switch (action)
		{
			case EXIT:
			exit = true;
			break;

			case RESIZE:
			resize_app(&app);
			show_tdirlist(d, &app);
			refresh_app(&app);
			break;
		}
	}
	destroy_app(&app);
	free_tdirlist(d);

}
