#include "main_audio.h"
#include "log/src/log.h"

#define MIN_FREQ 21
#define MAX_FREQ 22050


int audio_setup(t_audio_info* audio_info)
{
	audio_info->parameters = new_audio_parameters(MIN_FREQ, MAX_FREQ);

	log_info("Leyendo de a %d samples a %d Hz (%.2f ms)",
		audio_info->parameters.fft_samples,
		audio_info->parameters.min_sample_rate,
		audio_info->parameters.fft_duration_ms
	);

	// En Linux, para listar devices: arecord -L
	// audio_info->audio_in = open_audio_file("audios/sweep_log.wav");
	audio_info->audio_in = open_audio_device(audio_info->config.audio_driver, audio_info->config.audio_device, audio_info->parameters.min_sample_rate);

	if (!audio_info->audio_in.stream) {
		log_error("Error abriendo audio.");
		return 0;
	}

	audio_info->chunk.samples = (int16_t*)malloc(sizeof(int16_t) * audio_info->parameters.fft_samples);

	size_t fft_mem_size = sizeof(double) * audio_info->parameters.fft_samples;
	audio_info->fft.real = (double*)malloc(fft_mem_size);
	audio_info->fft.imaginary = (double*)malloc(fft_mem_size);
	audio_info->fft.length = 0;
	audio_info->fft.sample_rate = (double)audio_info->parameters.min_sample_rate;

	return 1;
}

int audio_loop(t_audio_info* audio_info)
{
	if (audio_info->chunk.length >= audio_info->parameters.fft_samples) {
		// log_debug("Limpio chunk (%d)", audio_info->chunk.length);
		// Solamente cambio el length, y voy reusando el mismo espacio de memoria
		audio_info->chunk.length = 0;
	}

	// Leo solamente los que me faltan para completar la FFT
	read_audio(audio_info->audio_in.stream, &audio_info->chunk, audio_info->parameters.fft_samples);

	if (feof(audio_info->audio_in.stream)) {
		// log_warn("Fin de stream");
	}

	if (ferror(audio_info->audio_in.stream)) {
		// log_error("Error de stream");
	}

	if (audio_info->chunk.length < audio_info->parameters.fft_samples) {
		// log_debug("Datos insuficientes para FFT (%d)", audio_info->chunk.length);
		// Sigo en la próxima
		return 1;
	}

	// log_debug("Calculo FFT (%d samples = %.2f ms)", audio_info->parameters.fft_samples, audio_info->parameters.fft_duration_ms);

	// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
	for (int i = 0; i < audio_info->parameters.fft_samples; i++) {
		// No importa si los valores son negativos.
		// Un corrimiento igual para todos los valores no altera el resultado de la FFT.
		audio_info->fft.real[i] = (double)(audio_info->chunk.samples[i]);
		audio_info->fft.imaginary[i] = 0;
	}

	audio_info->fft.length = audio_info->parameters.fft_samples;

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
