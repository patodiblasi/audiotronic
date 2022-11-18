#include "read_audio.h"

// Ejecuta un comando en un nuevo proceso, y redirige los descriptores fds
// hacia nuevos pipes, que posteriormente son retornados en el mismo array.
// En caso de error, retorna 0.

// Ver:
// https://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/
// https://www.gnu.org/software/libc/manual/html_node/Low_002dLevel-I_002fO.html
// Inspirado levemente en código original de popen.
int multi_popen_fds(int fds[], int fds_length, char command[])
{
	// Descriptores de los pipes a los que voy a redirigir.
	// [0] lectura, [1] escritura
	int new_fds[fds_length][2];

	// Creo un pipe por cada file descriptor:
	for (int i=0; i<fds_length; i++) {
		if (pipe(new_fds[i]) != 0) {
			// Error: cierro los abiertos
			for (int j=0; j<i; j++) {
				close(new_fds[j][0]);
				close(new_fds[j][1]);
			}
			return 0;
		}
	}

	// Duplico el proceso actual
	if (vfork() == 0) {
		// Estoy en el proceso hijo

		for (int i=0; i<fds_length; i++) {
			// Redirijo los descriptores de escritura:
			if (dup2(new_fds[i][1], fds[i]) == -1) {
				// Error en el proceso hijo:
				fprintf(stderr, "\nmulti_popen_fds: dup2: No fue posible redirigir los descriptores");
				exit(1);
			}
			// Cierro descriptores que ya no necesito (lectura y escritura):
			close(new_fds[i][0]);
			close(new_fds[i][1]);
		}

		// TODO: idealmente debería liberar el resto de los recursos
		// explícitamente, aunque igual se liberan al hacer execv
		// https://stackoverflow.com/questions/5429141/what-happens-to-malloced-memory-after-exec-changes-the-program-image

		// Reemplazo el proceso actual con el comando a ejecutar:
		char *argv[] = { "audiotronic_ffmpeg", "-c", command, NULL };
		execv("/bin/sh", argv);
		exit(0);
	} else {
		// Estoy en el proceso padre

		for (int i=0; i<fds_length; i++) {
			// Cierro descriptores que ya no necesito (escritura):
			close(new_fds[i][1]);

			// Agrego al resultado los nuevos descriptores de lectura:
			fds[i] = new_fds[i][0];
		}
	}

	return 1;
}

t_stream open_audio_file(const char* filename)
{
	t_stream stream;
	char command[200] = "";
	char command_format[100] = "ffmpeg";

	if (!SHOW_FFMPEG_OUTPUT) {
		strcat(command_format, " -loglevel quiet");
	}
	strcat(command_format, " -i %s -f s16le -ac 1 -");

	sprintf(command, command_format, filename);

	printf("\nAbriendo pipes SDTOUT y STDERR: %s\n", command);

	int fds[] = { STDOUT_FILENO, STDERR_FILENO };

	multi_popen_fds(fds, 2, command);

	stream.stream = fdopen(fds[0], "r");
	stream.errors = fdopen(fds[1], "r");

	return stream;
}

t_stream open_audio_device(const char* device, const char* filename, int sample_rate)
{
	// Captura del mic Plantronics:
	// ffmpeg -f alsa -sample_rate 44100 -i front:CARD=USB,DEV=0 -t 30 out.wav
	t_stream stream;
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

	int fds[] = { STDOUT_FILENO, STDERR_FILENO };

	multi_popen_fds(fds, 2, command);

	stream.stream = fdopen(fds[0], "r");
	stream.errors = fdopen(fds[1], "r");

	return stream;
}

void close_audio(t_stream* stream)
{
	printf("\nCerrando stream\n");

	fflush(stream->stream);
	pclose(stream->stream);
	stream->stream = NULL;

	fflush(stream->errors);
	pclose(stream->errors);
	stream->errors = NULL;
}

t_wave read_audio(FILE* fp, int samples_count)
{
	// Ver: fileno(fp) para obtener el file descriptor, y fcntl para hacer no bloqueante la lectura
	// https://www.geeksforgeeks.org/non-blocking-io-with-pipes-in-c/
	t_wave chunk;
	chunk.samples = (int16_t*)malloc(sizeof(int16_t) * samples_count);
	chunk.length = (int)fread(chunk.samples, 2, samples_count, fp);

	return chunk;
}
