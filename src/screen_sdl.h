#ifndef IS_SCREEN_SDL_INCLUDED
#define IS_SCREEN_SDL_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include "screen_types.h"

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Surface* logo;
	int width;
	int height;
	int is_ok;
} sdl_screen;

int screen_sdl_start();
int screen_sdl_end();
int screen_sdl_loop(t_screen_data* data);

sdl_screen start_screen(int width, int height);

#endif
