#ifndef READ_AUDIO_INCLUDED
#define READ_AUDIO_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	unsigned int length;
	int16_t* samples;
} samples_chunk;

FILE* open_audio_file(const char* filename);
FILE* open_audio_device(const char* filename, unsigned int sample_rate);
void close_audio(FILE* fp);
samples_chunk read_audio(FILE* fp, unsigned int samples);

#endif