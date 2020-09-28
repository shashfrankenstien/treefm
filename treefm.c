#include <stdlib.h>
#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"
#include "preview.h"


typedef struct {
	char latest;
	char cmd[256];
	int len;
} cmd_t;

void cmd_add(cmd_t*, char);
void cmd_pop(cmd_t*);
void cmd_clear(cmd_t*);
int cmd_getint(cmd_t*);


void _fmt_fsize(long int, char*);
void show_row(tfile*, WINDOW*, int, int, int);
void show_tdirlist(tdirlist*, tree_app*);

e_browser_acts kbd_events(tdirlist**, tree_app*);
void vnavigate(tdirlist*, tree_app*, e_vertical, int);
void hnavigate(tdirlist**, tree_app*, e_horizontal);


// function impl
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
	const char* namefmt = (f->isdir && DIRNAME_WITH_SLASH) ? "%s/" : "%s";
	mvwprintw(win, curr, curc, namefmt, f->name);
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
	prv_show_preview(d, app);
}



void vnavigate(tdirlist* d, tree_app* app, e_vertical direction, int step)
{
	// reset color highlight on current position
	int cp = get_tfile_colorpair(d, d->curs_pos);
	int gatp = A_NORMAL;

	if (cp!=NORM_COLOR)
		gatp |= A_BOLD;
	mvwchgat(app->browser, app->brw_props.curs_pos, 0, -1, gatp, cp, NULL); // reset color

	for (int i=1; i<=step; i++) {
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
	}

	mvwchgat(app->browser, app->brw_props.curs_pos, 0,
		-1, A_REVERSE|A_BOLD,
		get_tfile_colorpair(d, d->curs_pos), NULL); // highlight new cursor position

	char count[10];
	snprintf(count, 10, "%d/%d", d->curs_pos+1, d->files_count);
	write_cmd(app, count, RIGHT);
	prv_show_preview(d, app);
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
	prv_show_preview(*d, app);
}



void cmd_add(cmd_t* m, char c)
{
	m->latest = c;
	m->cmd[m->len] = c;
	m->cmd[++m->len] = '\0';
}

void cmd_pop(cmd_t* m)
{
	m->cmd[--m->len] = '\0';
}

void cmd_clear(cmd_t* m)
{
	m->cmd[0] = '\0';
	m->len = 0;
}

int cmd_getint(cmd_t* m)
{
	if (m->len == 0)
		return -1;

	for (int i=0; i<m->len; i++)
	{
		if (m->cmd[i] > '9' && m->cmd[i] < '0')
			return -1;
	}
	return atoi(m->cmd);
}


int cmd_mode(cmd_t* m, tree_app* app)
{
	curs_set(1);
	int ch = ':';
	do{
		switch(ch)
		{
			case 27: // escape (delayed)
			cmd_clear(m);
			return -1;
			break;

			case 127:
			case KEY_BACKSPACE:
			if (m->len>0)
			{
				cmd_pop(m);
				mvwdelch(app->cmd, 0, m->len);
			}
			break;

			default:
			mvwaddch(app->cmd, 0, m->len, ch);
			cmd_add(m, ch);
			break;
		}
		wrefresh(app->cmd);
	} while((ch = getch()) != 10);

	curs_set(0);
	return 0;
}


e_browser_acts kbd_events(tdirlist** d, tree_app* app)
{
	cmd_t command = {'\0', "", 0};

	int ch, n;
	char* disp;
	while((ch = getch()) != KEY_F(1))
	{
		switch(ch)
		{
			case KEY_RESIZE:
			return RESIZE;
			break;

			case KEY_LEFT:
			case 'h':
			cmd_clear(&command);
			break;

			case KEY_RIGHT:
			case 'l':
			hnavigate(d, app, RIGHT);
			cmd_clear(&command);
			break;

			case KEY_UP:
			case 'k':
			n = cmd_getint(&command);
			if (n < 0) n = 1;
			vnavigate(*d, app, UP, n);
			cmd_clear(&command);
			break;

			case KEY_DOWN:
			case 'j':
			n = cmd_getint(&command);
			if (n < 0) n = 1;
			vnavigate(*d, app, DOWN, n);
			cmd_clear(&command);
			break;

			case '0' ... '9':
			cmd_add(&command, ch);
			break;

			case ':':
			cmd_mode(&command, app);
			if (strcmp(command.cmd, ":q")==0)
				return EXIT;
			cmd_clear(&command);
			ch = '\0';
			break;
		}

		disp = (command.len > 0) ? &command.cmd[0] : (char*)&ch;
		write_cmd(app, disp, LEFT);
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
