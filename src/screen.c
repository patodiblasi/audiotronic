#include "screen.h"
#include "screen_text.h"
#include "screen_ncurses.h"
#include "screen_sdl.h"


int _screen_output_mode = SCREEN_MODE_TEXT;

int screen_start()
{
	switch (_screen_output_mode) {
		case SCREEN_MODE_TEXT:
			return screen_text_start();
		case SCREEN_MODE_NCURSES:
			return screen_ncurses_start();
		case SCREEN_MODE_SDL:
			return screen_sdl_start();
	}

	return 0;
}

int screen_end()
{
	switch (_screen_output_mode) {
		case SCREEN_MODE_TEXT:
			return screen_text_end();
		case SCREEN_MODE_NCURSES:
			return screen_ncurses_end();
		case SCREEN_MODE_SDL:
			return screen_sdl_end();
	}

	return 0;
}

int screen_loop(t_screen_data* data)
{
	switch (_screen_output_mode) {
		case SCREEN_MODE_TEXT:
			return screen_text_loop(data);
		case SCREEN_MODE_NCURSES:
			return screen_ncurses_loop(data);
		case SCREEN_MODE_SDL:
			return screen_sdl_loop(data);
	}

	return 0;
}

void screen_set_mode(int mode)
{
	_screen_output_mode = mode;
}
