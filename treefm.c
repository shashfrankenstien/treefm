#include <stdlib.h> /*malloc, free,..*/
#include <string.h> /*strlen*/
#include <curses.h>
#include <sys/stat.h> /*struct stat*/

#include "utils.h" /*macros*/
#include "dirlist.h"



typedef enum {UP, DOWN} e_vertical;
typedef enum {
	RIGHT=1,
	NEXT=1,
	OPEN=1,
	LEFT=0,
	PREV=0,
	CLOSE=0
} e_horizontal;

typedef struct twinprops {
	int rows, cols;
	int startr, startc;
	int padr, padc;
} twinprops;

typedef struct tree_app {
	WINDOW* browser;
	WINDOW* cmd;
	WINDOW* preview;

	twinprops root_props;
	twinprops brw_props;
	twinprops cmd_props;
	twinprops preview_props;

	char cmdtxt_l[100];
	char cmdtxt_r[100];
	char header_l[100];
	char header_r[100];
	char footer_l[100];
	char footer_r[100];
} tree_app;



// function declarations
void init_curses();
int create_app(tree_app* app);
void refresh_app(tree_app* app);
void resize_app(tree_app* app);
void destroy_app(tree_app* app, tdirlist*);

WINDOW* create_win(int, int, int, int);
WINDOW* create_win_from_props(twinprops* props);
void destroy_win(WINDOW*);

void _fmt_fsize(long int, char*);
void _write_cmd(tree_app*, char*, e_horizontal);
void _write_header(tree_app*, char*, e_horizontal);
void _write_footer(tree_app*, char*, e_horizontal);

void show_row(tfile*, WINDOW*, int, int, int);
void show_tdirlist(tdirlist*, tree_app*);
int kbd_events(tdirlist*, tree_app*);
void vnavigate(tdirlist*, tree_app*, e_vertical);
void hnavigate(tdirlist**, tree_app*, e_horizontal);


#include "config.h"


// function implementations
WINDOW* create_win(int height, int width, int starty, int startx)
{
	WINDOW* win;
	win = newwin(height, width, starty, startx);
	wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(win);
	return win;
}

WINDOW* create_win_from_props(twinprops* props)
{
	WINDOW* win;
	win = create_win(
		props->rows, props->cols,
		props->startr, props->startc
	);
	return win;
}

void destroy_win(WINDOW *win)
{
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners
	 * and so an ugly remnant of window.
	 */
	wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window
	 * 3. rs: character to be used for the right side of the window
	 * 4. ts: character to be used for the top side of the window
	 * 5. bs: character to be used for the bottom side of the window
	 * 6. tl: character to be used for the top left corner of the window
	 * 7. tr: character to be used for the top right corner of the window
	 * 8. bl: character to be used for the bottom left corner of the window
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(win);
	delwin(win);
}


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
	erase();
	werase(app->browser);
	werase(app->cmd);
	int maxc = app->brw_props.cols - (2*app->brw_props.padc);
	int curc = app->brw_props.startc + app->brw_props.padc;
	int padr = app->brw_props.padr;

	for (int r=0; r < d->files_count; r++)
		show_row(&d->files[r], app->browser, r+padr, curc, maxc);

	mvwchgat(app->browser, d->curs_pos+padr, 0,
		-1, A_REVERSE|A_BOLD,
		get_tfile_colorpair(d, d->curs_pos), NULL);

	_write_header(app, d->cwd, LEFT);
	_write_footer(app, d->cwd, LEFT);
	char count[10];
	snprintf(count, 10, "%d/%d", d->curs_pos+1, d->files_count);
	_write_cmd(app, count, RIGHT);
}


void vnavigate(tdirlist* d, tree_app* app, e_vertical vert)
{
	int cpos;
	if (vert==DOWN)
		cpos = iMIN(d->curs_pos + 1, (d->files_count - 1));
	else
		cpos = iMAX(d->curs_pos - 1, 0);

	int cp = get_tfile_colorpair(d, d->curs_pos);
	int gatp = A_NORMAL;
	if (cp!=NORM_COLOR)
		gatp |= A_BOLD;

	int padr = app->brw_props.padr; // vertical padding from config
	mvwchgat(app->browser, d->curs_pos+padr, 0, -1, gatp, cp, NULL); // reset color on prev position
	d->curs_pos = cpos;
	mvwchgat(app->browser, cpos+padr, 0,
		-1, A_REVERSE|A_BOLD,
		get_tfile_colorpair(d, cpos), NULL); // highlight new cursor position

	char count[10];
	snprintf(count, 10, "%d/%d", d->curs_pos+1, d->files_count);
	_write_cmd(app, count, RIGHT);
}

void hnavigate(tdirlist** d, tree_app* app, e_horizontal hor)
{
	tfile curfile = (*d)->files[(*d)->curs_pos];
	if (curfile.isdir)
	{
		tdirlist* new_d = listdir((const char*)curfile.fullpath);
		_write_header(app, strerror(new_d->err), RIGHT);
		if (new_d->err==0) {
			free_tdirlist(*d);
			*d = new_d;
			show_tdirlist(*d, app);
		}
		else {
			free_tdirlist(new_d);
		}
	}
}

int kbd_events(tdirlist* d, tree_app* app)
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
			hnavigate(&d, app, RIGHT);
			break;

			case KEY_UP:
			case 'k':
			vnavigate(d, app, UP);
			break;

			case KEY_DOWN:
			case 'j':
			vnavigate(d, app, DOWN);
			break;
		}
		char* c = (char*)&ch;
		_write_cmd(app, c, LEFT);
		refresh_app(app);
	}
	return EXIT;
}


void _write_cmd(tree_app* app, char* txt, e_horizontal align)
{
	if (align==RIGHT)
		strcpy(app->cmdtxt_r, txt);
	else
		strcpy(app->cmdtxt_l, txt);

	int startr, maxc, padc;
	maxc = app->cmd_props.cols;
	padc = app->cmd_props.padc;
	startr = maxc-strlen(app->cmdtxt_r)-padc;
	wmove(app->cmd, 0, 0);
	wclrtoeol(app->cmd);

	mvwprintw(app->cmd, 0, 0, "%s", app->cmdtxt_l);
	mvwprintw(app->cmd, 0, startr, "%s", app->cmdtxt_r);
}


void _write_header(tree_app* app, char* txt, e_horizontal align)
{
	if (align==RIGHT)
		strcpy(app->header_r, txt);
	else
		strcpy(app->header_l, txt);

	int startr, maxc, padc;
	maxc = app->root_props.cols;
	padc = app->cmd_props.padc;
	startr = maxc-strlen(app->header_r)-padc;
	wmove(stdscr, 0, 0);
	wclrtoeol(stdscr);

	mvwprintw(stdscr, 0, padc, "%s", app->header_l);
	mvwprintw(stdscr, 0, startr, "%s", app->header_r);
}

void _write_footer(tree_app* app, char* txt, e_horizontal align)
{
	if (align==RIGHT)
		strcpy(app->footer_r, txt);
	else
		strcpy(app->footer_l, txt);

	int startr, maxc, padc, row;
	maxc = app->root_props.cols;
	padc = app->cmd_props.padc;
	row = app->cmd_props.startr - 1;
	startr = maxc-strlen(app->footer_r)-padc;
	wmove(stdscr, row, 0);
	wclrtoeol(stdscr);

	mvwprintw(stdscr, row, padc, "%s", app->footer_l);
	mvwprintw(stdscr, row, startr, "%s", app->footer_r);
}


void init_curses()
{
	initscr(); //init ncurses
	cbreak(); //getch without <CR>
	keypad(stdscr, TRUE); //enable fn and arrow keys
	curs_set(0); //0, 1 or 2
	noecho(); //disable echo of user input

	start_color(); //enable colors
	init_pair(NORM_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(INVNORM_COLOR, COLOR_BLACK, COLOR_WHITE);
	init_pair(DIR_COLOR, COLOR_CYAN, COLOR_BLACK);
	init_pair(EXE_COLOR, COLOR_GREEN, COLOR_BLACK);

	init_color(COLOR_GREY, 400, 400, 400); //custom color
	init_pair(BG_COLOR, COLOR_WHITE, COLOR_GREY);
	clear();
}

int create_app(tree_app* app)
{
	init_curses(); // start root window

	twinprops rp = { //root
		.rows = LINES,
		.cols = COLS,
	};

	twinprops bp = { //browser
		.rows = LINES - (2*main_border_r) - 1, /*-1 for command window*/
		.cols = COLS - (2*main_border_c),
		.startr = main_border_r,
		.startc = main_border_c,
		.padr = intern_pad_r,
		.padc = intern_pad_c
	};

	twinprops cp = { //command
		.rows = 1,
		.cols = COLS,
		.startr = LINES-1,
		.startc = 0,
		.padc = main_border_c,
		.padr = 0
	};

	if (main_light_borders) wbkgd(stdscr, A_BOLD|COLOR_PAIR(INVNORM_COLOR));
	wborder(stdscr, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	refresh();

	app->root_props = rp;
	app->browser = create_win_from_props(&bp);
	app->cmd = create_win_from_props(&cp);
	app->brw_props = bp;
	app->cmd_props = cp;

	return 0;
}


void refresh_app(tree_app* app)
{
	refresh();
	wrefresh(app->browser);
	wrefresh(app->cmd);
}


void resize_app(tree_app* app)
{
	int newr, newc, deltar, deltac;
	getmaxyx(stdscr, newr, newc);
	deltar = newr - app->root_props.rows;
	deltac = newc - app->root_props.cols;

	app->root_props.cols += deltac;
	app->brw_props.cols += deltac;
	app->cmd_props.cols += deltac;
	if (deltar != 0 )
	{
		app->root_props.rows += deltar;
		app->brw_props.rows += deltar;
		app->cmd_props.startr += deltar;
		mvwin(app->cmd, app->cmd_props.startr, 0);
	}

	wresize(app->cmd, app->cmd_props.rows, app->cmd_props.cols);
	wresize(app->browser, app->brw_props.rows, app->brw_props.cols);

	char sz[10];
	snprintf(sz, 10, "%d,%d", newr, newc);
	_write_header(app, sz, RIGHT);
}


void destroy_app(tree_app* app, tdirlist* dlist)
{
	destroy_win(app->browser);
	destroy_win(app->cmd);
	refresh(); //paint screen
	endwin(); //end curses mode
	free_tdirlist(dlist);
	curs_set(2); //0, 1 or 2
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
		action = kbd_events(d, &app);
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
	destroy_app(&app, d);
}
