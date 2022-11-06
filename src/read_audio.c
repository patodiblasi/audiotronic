#include "read_audio.h"

FILE* open_audio(const char* filename)
{
	char* command;
	asprintf(&command, "ffmpeg -loglevel quiet -i %s -f s16le -ac 1 -", filename);
	printf("\nAbriendo archivo: %s\n", filename);

	// Open WAV file with FFmpeg and read raw samples via the pipe.
	FILE* fp = popen(command, "r");
	free(command);
	return fp;
}

void close_audio(FILE* fp)
{
	printf("\nCerrando archivo\n");
	fflush(fp);
	pclose(fp);
}

samples_chunk read_audio(FILE* fp, unsigned int samples_count)
{
	samples_chunk chunk;
	chunk.samples = (int16_t*)malloc(sizeof(int16_t) * samples_count);
	chunk.length = (unsigned int)fread(chunk.samples, 2, samples_count, fp);

	return chunk;
}
