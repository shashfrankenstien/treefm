#include <string.h> /*strlen*/
#include <curses.h>
#include <sys/stat.h> /*struct stat*/

#include "utils.h" /*macros*/
#include "dirlist.h"

//config defaults
int main_border_r = 1;
int main_border_c = 2;
int intern_pad_c = 1;

// function signatures
WINDOW* create_win(int, int, int, int);
void destroy_win(WINDOW*);

void _fmt_fsize(long int, char*);
void show_tdirlist(tdirlist*, WINDOW*, int, int);
void show_row(tfile, WINDOW*, int, int, int, int);
//void navigate(tdirlist* d, tnav nav);


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

void show_row(tfile f, WINDOW* win, int curr, int curc, int maxr, int maxc) 
{
	if (f._color_pair!=NORM_COLOR)
		wattron(win, A_BOLD | COLOR_PAIR(f._color_pair));
	
	char sz[] = "00000";
	_fmt_fsize(f.st->st_size, sz);
	mvwprintw(win, curr, curc, "%s", f.name);
	mvwprintw(win, curr, maxc-strlen(sz), "%s", sz);
	
	if (f._color_pair!=NORM_COLOR)
		wattroff(win, A_BOLD | COLOR_PAIR(f._color_pair));
}

void show_tdirlist(tdirlist* d, WINDOW* win, int starty, int startx)
{
	int maxr, maxc, curr, curc;
	getmaxyx(win, maxr, maxc);
	maxc -= (2*intern_pad_c);
	
	curc = startx + intern_pad_c;
	curr = starty;
	
	for (int i=0; i < (d->dirs_count+d->files_count); i++)
	{
		show_row(d->files[i], win, curr, curc, maxr, maxc);
		curr++;
	}
	mvwchgat(win, d->curs_pos, 0, -1, A_REVERSE, 0, NULL);
	wrefresh(win);
}



int main(int argc, char* argv[])
{
	const char* p;
	if (argc>1)
		p = argv[1];
	else
		p = ".";

	tdirlist* d = listdir(p);
	
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
	
	init_color(COLOR_GREY, 400, 400, 400);
	init_pair(BG_COLOR, COLOR_WHITE, COLOR_GREY);
	clear();

	int height = LINES - (2*main_border_r);
	int width = COLS - (2*main_border_c);
	int starty = main_border_r;
	int startx = main_border_c;
	
	wbkgd(stdscr, COLOR_PAIR(INVNORM_COLOR));
	wborder(stdscr, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wprintw(stdscr, 0, 0, "%s", p);
	refresh();

	WINDOW* my_win;
	my_win = create_win(height, width, starty, startx);

	
	d->curs_pos = 0;
	show_tdirlist(d, my_win, 0, 0);

	int ch, cpos;
	while((ch = getch()) != KEY_F(1))
	{	
		switch(ch)
		{	
			case KEY_LEFT:
			case 'h':
			break;

			case KEY_RIGHT:
			case 'l':
			break;

			case KEY_UP:
			case 'k':
			cpos = iMAX(d->curs_pos - 1, 0);
			mvwchgat(my_win, d->curs_pos, 0, -1, A_NORMAL, 0, NULL);
			d->curs_pos = cpos;
			mvwchgat(my_win, cpos, 0, -1, A_REVERSE, 0, NULL);
			break;

			case KEY_DOWN:
			case 'j':
			cpos = iMIN(d->curs_pos + 1, (d->dirs_count + d->files_count - 1));
			mvwchgat(my_win, d->curs_pos, 0, -1, A_NORMAL, 0, NULL);
			d->curs_pos = cpos;
			mvwchgat(my_win, cpos, 0, -1, A_REVERSE, 0, NULL);
			break;	
		}
		wrefresh(my_win);
	}
	destroy_win(my_win);
	refresh(); //paint screen
	//ch = getch();
	endwin(); //end curses mode

	curs_set(2); //0, 1 or 2
	free_tdirlist(d);
	printf("%c\n", ch);
}
