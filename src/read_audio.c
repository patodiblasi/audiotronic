#include "read_audio.h"

FILE* open_audio_file(const char* filename)
{
	char command[200] = "";
	char command_format[100] = "ffmpeg";

	if (!SHOW_FFMPEG_OUTPUT) {
		strcat(command_format, " -loglevel quiet");
	}
	strcat(command_format, " -i %s -f s16le -ac 1 -");

	sprintf(command, command_format, filename);

	printf("\nAbriendo pipe: %s\n", command);
	return popen(command, "r");
}

FILE* open_audio_device(const char* device, const char* filename, unsigned int sample_rate)
{
	// Captura del mic Plantronics:
	// ffmpeg -f alsa -sample_rate 44100 -i front:CARD=USB,DEV=0 -t 30 out.wav
	char command[200] = "";
	char command_format[100] = "ffmpeg";
	char command_device_options[100] = "";

	if (!SHOW_FFMPEG_OUTPUT) {
		strcat(command_format, " -loglevel quiet");
	}

	if (strcmp(device, "alsa") == 0) {
		// alsa permite opción sample_rate
		sprintf(command_device_options, " -f %s -sample_rate %d", device, sample_rate);
	} else {
		// avfoundation no permite opción sample_rate
		sprintf(command_device_options, " -f %s", device);
	}
	// TODO: otros?

	strcat(command_format, command_device_options);
	strcat(command_format, " -i %s -t 30 -f s16le -ac 1 -");

	sprintf(command, command_format, filename);

	printf("\nAbriendo pipe: %s\n", command);
	return popen(command, "r");
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
