#ifndef IS_SCREEN_INCLUDED
#define IS_SCREEN_INCLUDED

#include "screen_types.h"

#define SCREEN_MODE_TEXT 0
#define SCREEN_MODE_NCURSES 1
#define SCREEN_MODE_SDL 2

int screen_start();
int screen_loop(t_screen_data* data);
int screen_end();

void screen_set_mode(int mode);

#endif