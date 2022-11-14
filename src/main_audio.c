#include "main_audio.h"
#include "show_audio.h"

#define MIN_FREQ 24
#define MAX_FREQ 24576
#define AUDIO_DRIVER "alsa"
#define AUDIO_INPUT_DEVICE "front:CARD=USB,DEV=0"

// En Linux, para listar devices: arecord -L

int audio_setup(t_audio_info* audio_info)
{
	audio_info->empty_stream_count = 0;
	audio_info->config = new_audio_config(MIN_FREQ, MAX_FREQ);

	printf("Leyendo de a %d samples a %d Hz (%.2f ms)",
		audio_info->config.min_samples,
		audio_info->config.min_sample_rate,
		audio_info->config.min_samples_duration_ms
	);

	// audio_info->fp = open_audio_file("audios/sweep.mp3");
	audio_info->fp = open_audio_device(AUDIO_DRIVER, AUDIO_INPUT_DEVICE, audio_info->config.min_sample_rate);
	if (!audio_info->fp) {
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

	// TODO: tendría que asegurar que sea potencia de 2 antes de hacer cálculos
	audio_info->fft.length = audio_info->chunk.length;

	// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
	for (unsigned int i = 0; i < audio_info->chunk.length; i++) {
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

	close_audio(audio_info->fp);
}
