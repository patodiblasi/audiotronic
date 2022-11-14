#ifndef IS_MAIN_AUDIO_INCLUDED
#define IS_MAIN_AUDIO_INCLUDED

#include "read_audio.h"
#include "process_audio.h"

// TODO: refactor de audio_info para usar t_fft
typedef struct {
	t_wave chunk;
	t_fft fft;
	int empty_stream_count = 0;
	FILE* fp;
	audio_config config;
} t_audio_info;

int audio_setup(t_audio_info* audio_info);
int audio_loop(t_audio_info* audio_info);
void audio_end(t_audio_info* audio_info);

#endif
