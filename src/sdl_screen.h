#ifndef IS_SDL_SCREEN_INCLUDED
#define IS_SDL_SCREEN_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Surface* logo;
	int width;
	int height;
	int is_ok;
} sdl_screen;

sdl_screen create_screen(int width, int height)
{
	sdl_screen screen;
	screen.width = width;
	screen.height = height;
	screen.is_ok = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	// Set texture filtering to linear
	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		fprintf(stderr, "SDL_SetHint: %s\n", SDL_GetError());
	}

	// Create window
	screen.window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen.width, screen.height, SDL_WINDOW_SHOWN);
	if(screen.window == NULL) {
		fprintf(stderr, "SDL_Window: %s\n", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED);
	if(screen.renderer == NULL) {
		fprintf(stderr, "SDL_Renderer: %s\n", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	screen.surface = SDL_GetWindowSurface(screen.window);
	if(screen.renderer == NULL) {
		fprintf(stderr, "SDL_GetWindowSurface: %s\n", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(screen.renderer, 0x00, 0x00, 0x00, 0x00);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if(!(IMG_Init(imgFlags) & imgFlags)) {
		fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
		screen.is_ok = false;
		return screen;
	}

	return screen;
}

void close_screen(sdl_screen screen)
{
	//Destroy window
	SDL_DestroyRenderer(screen.renderer);
	SDL_DestroyWindow(screen.window);
	SDL_FreeSurface(screen.surface);
	screen.renderer = NULL;
	screen.window = NULL;
	screen.surface = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}


SDL_Surface* load_surface(sdl_screen screen, std::string path)
{
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL) {
		fprintf(stderr, "IMG_Load: %s\n", path.c_str(), IMG_GetError());
		return NULL;
	}

	//Convert surface to screen format
	SDL_Surface* optimizedSurface = SDL_ConvertSurface(loadedSurface, screen.surface->format, 0);
	if(optimizedSurface == NULL) {
		fprintf(stderr, "SDL_ConvertSurface: %s\n", path.c_str(), SDL_GetError());
		return NULL;
	}

	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	return optimizedSurface;
}

int load_screen(sdl_screen screen)
{
	// Logo
	screen.logo = load_surface(screen, "logo.png");
	if(screen.logo == NULL) {
		fprintf(stderr, "No se pudo cargar el logo\n");
		return false;
	}

	return true;
}

// Retorna bool indicando si continuar
bool screen_frame(sdl_screen screen, int16_t* signal, int signal_length, double* fft, int fft_length)
{
	//Event handler
	SDL_Event e;

	//Handle events on queue
	while(SDL_PollEvent(&e) != 0) {
		//User requests quit
		if(e.type == SDL_QUIT) {
			return false;
		}
	}

	double avg = 0;
	for (int i = 0; i < signal_length; i++) {
		avg += (double)signal[i];
	}

	avg = avg / (double)signal_length;
	printf("\nAVG: %f", avg);

	//Clear screen
	SDL_SetRenderDrawColor(screen.renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(screen.renderer);

	// Línea indicando amplitud
	int height = (screen.height / 2) * (1 + avg / 65535);
	SDL_SetRenderDrawColor(screen.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderDrawLine(screen.renderer, 0, height, screen.width, height);

	//Render red filled quad
	SDL_Rect fillRect = { screen.width / 4, screen.height / 4, screen.width / 2, screen.height / 2 };
	SDL_SetRenderDrawColor(screen.renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(screen.renderer, &fillRect);

	//Render green outlined quad
	SDL_Rect outlineRect = { screen.width / 6, screen.height / 6, screen.width * 2 / 3, screen.height * 2 / 3 };
	SDL_SetRenderDrawColor(screen.renderer, 0x00, 0xFF, 0x00, 0xFF);
	SDL_RenderDrawRect(screen.renderer, &outlineRect);

	//Draw vertical line of yellow dots
	SDL_SetRenderDrawColor(screen.renderer, 0xFF, 0xFF, 0x00, 0xFF);
	for(int i = 0; i < screen.height; i += 4) {
		SDL_RenderDrawPoint(screen.renderer, screen.width / 2, i);
	}

	//Update screen
	SDL_RenderPresent(screen.renderer);

	// //Apply the image stretched
	// SDL_Rect stretchRect;
	// stretchRect.x = 0;
	// stretchRect.y = 0;
	// stretchRect.w = screen.width;
	// stretchRect.h = screen.height;
	// SDL_BlitScaled(screen.logo, NULL, screen.surface, &stretchRect);
	//
	// //Update the surface
	// SDL_UpdateWindowSurface(screen.window);

	return true;
}

sdl_screen start_screen(int width, int height)
{
	sdl_screen screen = create_screen(width, height);

	if(screen.is_ok) {
		load_screen(screen);
	}

	return screen;
}

#endif