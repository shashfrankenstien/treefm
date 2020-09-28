#pragma once

#include "dirlist.h"
#include "ui.h"


int get_dirlist_text(tfile* f, char* content, int size_limit);
void prv_show_preview(tdirlist* d, tree_app* app);
