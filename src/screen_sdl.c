#include <stdio.h>
#include <string>
#include <cmath>
#include "screen_sdl.h"
#include "log/src/log.h"

int screen_sdl_start()
{
	log_info("Iniciando modo SDL");

	sdl_screen screen = start_screen(800, 600);
	if (!screen.is_ok) {
		return 0;
	}

	return 1;
}

int screen_sdl_end()
{
	log_info("Terminando modo SDL");
	return 1;
}

int screen_sdl_loop(t_screen_data* data)
{
	log_info("Loop modo SDL");
	return 1;
}

sdl_screen create_screen(int width, int height)
{
	sdl_screen screen;
	screen.width = width;
	screen.height = height;
	screen.is_ok = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		log_error("SDL_Init: %s", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	// Set texture filtering to linear
	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		log_error("SDL_SetHint: %s", SDL_GetError());
	}

	// Create window
	screen.window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen.width, screen.height, SDL_WINDOW_SHOWN);
	if(screen.window == NULL) {
		log_error("SDL_Window: %s", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED);
	if(screen.renderer == NULL) {
		log_error("SDL_Renderer: %s", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	screen.surface = SDL_GetWindowSurface(screen.window);
	if(screen.renderer == NULL) {
		log_error("SDL_GetWindowSurface: %s", SDL_GetError());
		screen.is_ok = false;
		return screen;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(screen.renderer, 0x00, 0x00, 0x00, 0x00);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if(!(IMG_Init(imgFlags) & imgFlags)) {
		log_error("IMG_Init: %s", IMG_GetError());
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
		log_error("IMG_Load: %s", path.c_str(), IMG_GetError());
		return NULL;
	}

	//Convert surface to screen format
	SDL_Surface* optimizedSurface = SDL_ConvertSurface(loadedSurface, screen.surface->format, 0);
	if(optimizedSurface == NULL) {
		log_error("SDL_ConvertSurface: %s", path.c_str(), SDL_GetError());
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
		log_error("No se pudo cargar el logo");
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
	log_debug("AVG: %f", avg);

	//Clear screen
	SDL_SetRenderDrawColor(screen.renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(screen.renderer);

	// L??nea indicando amplitud
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
