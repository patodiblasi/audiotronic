#ifndef IS_SHOW_AUDIO_INCLUDED
#define IS_SHOW_AUDIO_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include "read_audio.h"
#include "process_audio.h"

#define COLOR_PAIR_TEXT 1
#define COLOR_PAIR_ACCENT 2
#define COLOR_PAIR_PRIMARY 3
#define COLOR_PAIR_ERROR 4

void draw_wave(int16_t* samples, int length, int samples_per_line, double multiplier);
void print_wave_values(int16_t* samples, int length);

int ncurses_start();
void ncurses_end();
int ncurses_loop(t_fft* fft, t_stream* audio_in);

void delete_win(WINDOW* win);

void draw_logo(WINDOW* win);
void draw_fft(WINDOW* win, t_fft* fft, t_frequency_band_array* band_array);

#endif