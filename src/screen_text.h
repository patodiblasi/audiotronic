#ifndef IS_SCREEN_TEXT_INCLUDED
#define IS_SCREEN_TEXT_INCLUDED

#include <stdint.h>
#include "screen_types.h"

int screen_text_start();
int screen_text_end();
int screen_text_loop(t_screen_data* data);

void draw_wave(int16_t* samples, int length, int samples_per_line, double multiplier);
void print_wave_values(int16_t* samples, int length);

#endif