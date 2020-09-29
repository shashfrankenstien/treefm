#include <stdlib.h> /*malloc, free,..*/
#include <string.h> /*strlen*/
#include <curses.h>
#include <sys/stat.h> /*struct stat*/
#include <math.h> /*floor, ceil*/

#include "utils.h" /*macros*/
#include "ui.h"


void init_curses();
WINDOW* create_win(int, int, int, int);
WINDOW* create_win_from_props(twinprops* props);
void destroy_win(WINDOW*);


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



void write_cmd(tree_app* app, char* txt, e_horizontal align)
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


void write_header(tree_app* app, char* txt, e_horizontal align)
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

void write_footer(tree_app* app, char* txt, e_horizontal align)
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

	app->root_props = (twinprops){
		.rows = LINES,
		.cols = COLS,
	};

	int browser_width = ceil(COLS*BROWSER_OUTER_WIDTH);
	int preview_width = COLS - browser_width;

	app->preview_props = (twinprops){
		.rows = LINES - (2*MAIN_BORDER_R) - 1, /*-1 for command window*/
		.cols = preview_width - MAIN_BORDER_C,
		.startr = MAIN_BORDER_R,
		.startc = browser_width,
		.padc = INTERN_PAD_C,
	};

	app->brw_props = (twinprops){
		.rows = LINES - (2*MAIN_BORDER_R) - 1, /*-1 for command window*/
		.cols = browser_width - (2*MAIN_BORDER_C),
		.startr = MAIN_BORDER_R,
		.startc = MAIN_BORDER_C,
		.padc = INTERN_PAD_C,
		.curs_pos = 0
	};

	app->cmd_props = (twinprops){
		.rows = 1,
		.cols = COLS,
		.startr = LINES-1,
		.startc = 0,
		.padc = MAIN_BORDER_C,
	};

	if (MAIN_LIGHT_BORDERS) wbkgd(stdscr, A_BOLD|COLOR_PAIR(INVNORM_COLOR));
	wborder(stdscr, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	refresh();

	app->browser = create_win_from_props(&app->brw_props);
	app->cmd = create_win_from_props(&app->cmd_props);
	app->preview = create_win_from_props(&app->preview_props);
	app->cmdtxt_l[0] = '\0';
	app->cmdtxt_r[0] = '\0';
	app->header_l[0] = '\0';
	app->header_r[0] = '\0';
	app->footer_l[0] = '\0';
	app->footer_r[0] = '\0';
	return 0;
}



void resize_app(tree_app* app)
{
	int newr, newc, deltar, deltac;
	getmaxyx(stdscr, newr, newc);
	deltar = newr - app->root_props.rows;
	deltac = newc - app->root_props.cols;

	app->root_props.cols += deltac;
	app->cmd_props.cols += deltac;

	int main_width = app->brw_props.cols + app->preview_props.cols + deltac;
	app->brw_props.cols = ceil(main_width * BROWSER_OUTER_WIDTH);
	app->preview_props.cols = main_width - app->brw_props.cols;

	app->preview_props.startc = app->brw_props.cols + (3*MAIN_BORDER_C);
	mvwin(app->preview, app->preview_props.startr, app->preview_props.startc);

	if (deltar != 0 )
	{
		app->root_props.rows += deltar;
		app->brw_props.rows += deltar;
		app->preview_props.rows += deltar;

		app->cmd_props.startr += deltar;
		mvwin(app->cmd, app->cmd_props.startr, 0);
	}

	wresize(app->cmd, app->cmd_props.rows, app->cmd_props.cols);
	wresize(app->browser, app->brw_props.rows, app->brw_props.cols);
	wresize(app->preview, app->preview_props.rows, app->preview_props.cols);

	char sz[10];
	snprintf(sz, 10, "%d,%d", newr, newc);
	write_header(app, sz, RIGHT);
}


void refresh_app(tree_app* app)
{
	refresh();
	wrefresh(app->browser);
	wrefresh(app->cmd);
	wrefresh(app->preview);
}


void erase_app(tree_app* app)
{
	erase();
	werase(app->browser);
	werase(app->cmd);
	werase(app->preview);
}


void destroy_app(tree_app* app)
{
	destroy_win(app->browser);
	destroy_win(app->cmd);
	destroy_win(app->preview);
	refresh(); //paint screen
	endwin(); //end curses mode
	curs_set(2); //0, 1 or 2
}
