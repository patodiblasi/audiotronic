#ifndef IS_MAIN_AUDIO_INCLUDED
#define IS_MAIN_AUDIO_INCLUDED

#include "read_audio.h"
#include "process_audio.h"

typedef struct {
	samples_chunk chunk;
	double* real;
	double* imag;
	int empty_stream_count = 0;
	FILE* fp;
	audio_config config;
} t_audio_info;

int audio_setup(t_audio_info* audio_info);
int audio_loop_start(t_audio_info* audio_info);
int audio_loop_end(t_audio_info* audio_info);
void audio_end(t_audio_info* audio_info);

#endif
