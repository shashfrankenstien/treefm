#include <string.h>

#include "utils.h" /*macros*/
#include "dirlist.h"
#include "ui.h"



int kbd_events(tdirlist**, tree_app*);
void vnavigate(tdirlist*, tree_app*, e_vertical);
void hnavigate(tdirlist**, tree_app*, e_horizontal);



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
		_write_cmd(app, c, LEFT);
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
	destroy_app(&app, d);
}
