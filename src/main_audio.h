#ifndef IS_MAIN_AUDIO_INCLUDED
#define IS_MAIN_AUDIO_INCLUDED

#include "read_audio.h"
#include "process_audio.h"
#include "config.h"

typedef struct {
	t_wave chunk;
	t_fft fft;
	t_stream audio_in;
	t_audio_parameters parameters;
	t_audiotronic_config config;
} t_audio_info;

int audio_setup(t_audio_info* audio_info);
int audio_loop(t_audio_info* audio_info);
void audio_end(t_audio_info* audio_info);

int open_audio(t_audio_info* audio_info);
int read_audio(t_audio_info* audio_info);
int check_stream(FILE* stream);

#endif
