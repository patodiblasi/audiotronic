#ifndef IS_READ_AUDIO_INCLUDED
#define IS_READ_AUDIO_INCLUDED

#ifndef SHOW_FFMPEG_OUTPUT
	#define SHOW_FFMPEG_OUTPUT 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct {
	int length;
	int16_t* samples;
} t_wave;

FILE* open_audio_file(const char* filename);
FILE* open_audio_device(const char* device, const char* filename, int sample_rate);
void close_audio(FILE* fp);
t_wave read_audio(FILE* fp, int samples);

#endif
