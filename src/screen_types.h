#ifndef IS_SCREEN_TYPES_INCLUDED
#define IS_SCREEN_TYPES_INCLUDED

#include "read_audio.h"
#include "process_audio.h"

typedef struct {
	t_stream* audio_in;
	t_wave* wave;
	t_fft* fft;
} t_screen_data;

#endif