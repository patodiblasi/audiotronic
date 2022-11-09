#include "main_audio.h"
#include "show_audio.h"

#define MIN_FREQ 20
#define MAX_FREQ 22050
#define AUDIO_DRIVER "alsa"
#define AUDIO_INPUT_DEVICE "front:CARD=USB,DEV=0"

void audio_setup(t_audio_info* audio_info)
{
	audio_info->empty_stream_count = 0;
	audio_info->config = new_audio_config(MIN_FREQ, MAX_FREQ);

	printf("Leyendo de a %d samples a %d Hz (%.2f ms)",
		audio_info->config.min_samples,
		audio_info->config.min_sample_rate,
		audio_info->config.min_samples_duration_ms
	);

	// audio_info->fp = open_audio_file("audios/sentinel.wav");
	audio_info->fp = open_audio_device(AUDIO_DRIVER, AUDIO_INPUT_DEVICE, audio_info->config.min_sample_rate);
	if (!audio_info->fp) {
		fprintf(stderr, "\nError abriendo audio.\n");
	}
}

int audio_loop_start(t_audio_info* audio_info)
{
	audio_info->chunk = read_audio(audio_info->fp, audio_info->config.min_samples);
	if (audio_info->chunk.length == 0) {
		audio_info->empty_stream_count++;
		if (audio_info->empty_stream_count > 100) {
			printf("\n---------- FIN DE STREAM ----------\n");
			// Como corto este ciclo de loop, hago la limpieza acá
			free(audio_info->chunk.samples);
			audio_info->chunk.samples = NULL;
			return 0;
		}
	}

	audio_info->empty_stream_count = 0;

	draw_wave(audio_info->chunk.samples, audio_info->chunk.length);
	// print_wave_values(chunk.samples, chunk.length);

	// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
	size_t fft_size = sizeof(double) * audio_info->chunk.length;
	audio_info->real = (double*)malloc(fft_size);
	audio_info->imag = (double*)malloc(fft_size);

	for (unsigned int i = 0; i < audio_info->chunk.length; i++) {
		audio_info->real[i] = (double)(audio_info->chunk.samples[i]);
		audio_info->imag[i] = 0;
	}
	signal_to_fft(audio_info->real, audio_info->imag, audio_info->chunk.length, (double)audio_info->config.min_sample_rate);

	// Cada posición del array corresponde a una banda de frecuencia.
	// Todas las posiciones son del mismo ancho: sample_rate / length
	// Tener en cuenta que el oído no percibe linealmente las frecuencias
	// (una octava es el doble de frecuencia). Por esto, una frecuencia de
	// 100 Hz se parece a una de 200 Hz en igual medida que una de 10 KHz
	// se parece a otra de 20 KHz.

	////////////////////////////////////////////////////////////////////////
	// printf("\n-------------------------\nFFT:\n");
	// for (unsigned int i = 0; i < chunk.length; i++) {
	// 	printf("\n%d\t%d\t%.2f\t%.2f", i, chunk.samples[i], real[i], imag[i]);
	// }
	return 1;
}

int audio_loop_end(t_audio_info* audio_info)
{
	free(audio_info->chunk.samples);
	audio_info->chunk.samples = NULL;
	free(audio_info->real);
	audio_info->real = NULL;
	free(audio_info->imag);
	audio_info->imag = NULL;

	return 1;
}

void audio_end(t_audio_info* audio_info)
{
	close_audio(audio_info->fp);
}
