#include "main_audio.h"

#define MIN_FREQ 24
#define MAX_FREQ 24576

// En Linux, para listar devices: arecord -L

char* audio_driver = getenv("AUDIOTRONIC_DRIVER");
char* audio_input_device = getenv("AUDIOTRONIC_INPUT");

int audio_setup(t_audio_info* audio_info)
{
	audio_info->config = new_audio_config(MIN_FREQ, MAX_FREQ);

	printf("Leyendo de a %d samples a %d Hz (%.2f ms)",
		audio_info->config.min_samples,
		audio_info->config.min_sample_rate,
		audio_info->config.min_samples_duration_ms
	);

	// audio_info->audio_in = open_audio_file("audios/sweep_log.wav");
	audio_info->audio_in = open_audio_device(audio_driver, audio_input_device, audio_info->config.min_sample_rate);

	if (!audio_info->audio_in.stream) {
		fprintf(stderr, "\nError abriendo audio.\n");
		return 0;
	}

	// OJO: el input de la FFT tiene que ser en cantidades potencia de 2
	size_t fft_size = sizeof(double) * audio_info->config.min_samples;
	audio_info->fft.real = (double*)malloc(fft_size);
	audio_info->fft.imaginary = (double*)malloc(fft_size);
	audio_info->fft.length = 0;
	audio_info->fft.sample_rate = (double)audio_info->config.min_sample_rate;

	return 1;
}

int audio_loop(t_audio_info* audio_info)
{
	int is_stream_ok = 1;

	audio_info->chunk = read_audio(audio_info->audio_in.stream, audio_info->config.min_samples);

	if (feof(audio_info->audio_in.stream)) {
		fflush(stdout);
		fprintf(stderr, "\nFin de stream");
		is_stream_ok = 0;
	}
	if (ferror(audio_info->audio_in.stream)) {
		fflush(stdout);
		fprintf(stderr, "\nError de stream");
		is_stream_ok = 0;
	}
	if (!is_power_of_2(audio_info->chunk.length)) {
		fflush(stdout);
		fprintf(stderr, "\nEl segmento del stream no es potencia de 2");
		is_stream_ok = 0;
	}

	if (!is_stream_ok) {
		// Sigo en la próxima
		return 1;
	}

	audio_info->fft.length = audio_info->chunk.length;

	// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
	for (int i = 0; i < audio_info->chunk.length; i++) {
		// No importa si los valores son negativos.
		// Un corrimiento igual para todos los valores no altera el resultado de la FFT.
		audio_info->fft.real[i] = (double)(audio_info->chunk.samples[i]);
		audio_info->fft.imaginary[i] = 0;
	}

	signal_to_fft(&audio_info->fft);

	return 1;
}

void audio_end(t_audio_info* audio_info)
{
	free(audio_info->chunk.samples);
	audio_info->chunk.samples = NULL;
	free(audio_info->fft.real);
	audio_info->fft.real = NULL;
	free(audio_info->fft.imaginary);
	audio_info->fft.imaginary = NULL;

	close_audio(&audio_info->audio_in);
}

int is_power_of_2(int x)
{
	return (x & (x - 1)) == 0;
}
