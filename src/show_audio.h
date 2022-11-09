#ifndef IS_SHOW_AUDIO_INCLUDED
#define IS_SHOW_AUDIO_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void draw_wave(int16_t* samples, unsigned int length, unsigned int samples_per_line, double multiplier);
void print_wave_values(int16_t* samples, unsigned int length);

#endif