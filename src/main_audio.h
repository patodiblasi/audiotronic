#ifndef IS_MAIN_AUDIO_INCLUDED
#define IS_MAIN_AUDIO_INCLUDED

#include "read_audio.h"
#include "process_audio.h"

typedef struct {
	t_wave chunk;
	t_fft fft;
	t_stream audio_in;
	t_audio_parameters parameters;
} t_audio_info;

int audio_setup(t_audio_info* audio_info);
int audio_loop(t_audio_info* audio_info);
void audio_end(t_audio_info* audio_info);

#endif
