#ifndef IS_SHOW_AUDIO_INCLUDED
#define IS_SHOW_AUDIO_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define COLOR_PAIR_TEXT 1
#define COLOR_PAIR_ACCENT 2
#define COLOR_PAIR_PRIMARY 3
#define COLOR_PAIR_ERROR 4

void draw_wave(int16_t* samples, unsigned int length, unsigned int samples_per_line, double multiplier);
void print_wave_values(int16_t* samples, unsigned int length);

void screen_start();
void screen_draw_fft(double *fft_real, unsigned int fft_length, unsigned int fft_sample_rate, unsigned int bands);
void screen_end();

#endif