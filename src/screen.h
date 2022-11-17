#ifndef IS_SCREEN_INCLUDED
#define IS_SCREEN_INCLUDED

#include "process_audio.h"
#include "read_audio.h"

#define SCREEN_MODE_TEXT 0
#define SCREEN_MODE_NCURSES 1
#define SCREEN_MODE_SDL 2

int screen_start();
int screen_loop(t_fft* fft, t_stream* audio_in);
int screen_end();
void screen_set_mode(int mode);

#endif