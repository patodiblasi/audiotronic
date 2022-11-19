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
#include <unistd.h>

typedef struct {
	int length;
	int16_t* samples;
} t_wave;

typedef struct {
	FILE* stream;
	FILE* errors;
} t_stream;

int multi_popen_fds(int fds[], int fds_length, const char command[]);
t_stream open_audio_stream(char* command);
t_stream open_audio_file(const char* filename);
t_stream open_audio_device(const char* device, const char* filename, int sample_rate);
void close_audio(t_stream* stream);
size_t read_audio(FILE* fp, t_wave* chunk, int samples_to_read);

#endif
