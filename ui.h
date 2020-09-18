#pragma once

#include <curses.h>


typedef enum {
	EXIT,
	RESIZE,
} e_browser_acts;

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
void destroy_app(tree_app* app);

WINDOW* create_win(int, int, int, int);
WINDOW* create_win_from_props(twinprops* props);
void destroy_win(WINDOW*);

void _fmt_fsize(long int, char*);
void _write_cmd(tree_app*, char*, e_horizontal);
void _write_header(tree_app*, char*, e_horizontal);
void _write_footer(tree_app*, char*, e_horizontal);
