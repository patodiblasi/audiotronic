#ifndef IS_SCREEN_NCURSES_INCLUDED
#define IS_SCREEN_NCURSES_INCLUDED

#include <ncurses.h>
#include "read_audio.h"
#include "process_audio.h"

#define COLOR_PAIR_TEXT 1
#define COLOR_PAIR_ACCENT 2
#define COLOR_PAIR_PRIMARY 3
#define COLOR_PAIR_ERROR 4

int screen_ncurses_start();
int screen_ncurses_end();
int screen_ncurses_loop(t_fft* fft, t_stream* audio_in);

void delete_win(WINDOW* win);

void draw_logo(WINDOW* win);
void draw_fft(WINDOW* win, t_fft* fft, t_frequency_band_array* band_array);

#endif