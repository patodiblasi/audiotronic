#include "main_audio.h"
#include <errno.h>
#include "log/src/log.h"

#define MIN_FREQ 20
#define MAX_FREQ 22050
#define EOF_COUNT_BEFORE_RESTART 10

int _audio_eof_count = 0; // Cuenta la cantidad de EOF seguidos que tuve al leer el audio

int audio_setup(t_audio_info* audio_info)
{
	audio_info->parameters = new_audio_parameters(MIN_FREQ, MAX_FREQ);

	log_info("Leyendo de a %d samples a %d Hz (%.2f ms)",
		audio_info->parameters.fft_samples,
		audio_info->parameters.min_sample_rate,
		audio_info->parameters.fft_duration_ms
	);

	audio_info->chunk.samples = (int16_t*)malloc(sizeof(int16_t) * audio_info->parameters.fft_samples);

	size_t fft_mem_size = sizeof(double) * audio_info->parameters.fft_samples;
	audio_info->fft.real = (double*)malloc(fft_mem_size);
	audio_info->fft.imaginary = (double*)malloc(fft_mem_size);
	audio_info->fft.length = 0;
	audio_info->fft.sample_rate = (double)audio_info->parameters.min_sample_rate;

	return 1;
}

int check_stream(FILE* stream)
{
	int stream_ok = 1;

	if (feof(stream)) {
		stream_ok = 0;
		log_warn("Fin de stream: %d %s", errno, strerror(errno));
	}

	if (ferror(stream)) {
		stream_ok = 0;
		log_error("Error de stream: %d %s", errno, strerror(errno));
	}

	return stream_ok;
}

int open_audio(t_audio_info* audio_info)
{
	if (audio_info->audio_in.stream) {
		if (_audio_eof_count < EOF_COUNT_BEFORE_RESTART) {
			// Ya hay un stream abierto y responde bien, no hago nada
			return 1;
		} else {
			log_debug("Cerrando stream roto");
			// Cierro el stream, para abrirlo de nuevo
			close_audio(&audio_info->audio_in);
		}
	}

	log_debug("Abriendo stream");
	_audio_eof_count = 0;
	errno = 0;

	// En Linux, para listar devices: arecord -L
	// audio_info->audio_in = open_audio_file("audios/sweep_linear_10s.wav");
	// audio_info->audio_in = open_audio_file("audios/sweep_log_10s.wav");
	// audio_info->audio_in = open_audio_file("audios/440_1-1.wav");
	audio_info->audio_in = open_audio_device(audio_info->config.audio_driver, audio_info->config.audio_device, audio_info->parameters.min_sample_rate);

	return check_stream(audio_info->audio_in.stream);
}


int read_audio(t_audio_info* audio_info)
{
	if (audio_info->chunk.length >= audio_info->parameters.fft_samples) {
		log_trace("Limpiando chunk (%d)", audio_info->chunk.length);
		// Solamente cambio el length, y voy reusando el mismo espacio de memoria
		audio_info->chunk.length = 0;
	}

	errno = 0;

	// Leo solamente los que me faltan para completar la FFT
	int samples_to_read = audio_info->parameters.fft_samples - audio_info->chunk.length;
	log_trace("READ %d", samples_to_read);
	int size_read = (int)read_audio(audio_info->audio_in.stream, &audio_info->chunk, samples_to_read);
	log_trace("Samples leídos: %d", size_read);

	if (size_read == samples_to_read) {
		// Reseteo contador
		_audio_eof_count = 0;
	} else {
		if (feof(audio_info->audio_in.stream)) {
			_audio_eof_count++;
			log_trace("Fin de stream %d", _audio_eof_count);
		}
	}

	return 1;
}


int audio_loop(t_audio_info* audio_info)
{
	if (!open_audio(audio_info)) {
		// Sigo en la próxima
		return 1;
	}

	read_audio(audio_info);

	if (audio_info->chunk.length < audio_info->parameters.fft_samples) {
		// log_debug("Datos insuficientes para FFT (%d)", audio_info->chunk.length);
		// Sigo en la próxima
		return 1;
	}

	log_trace("Calculo FFT (%d samples = %.2f ms)", audio_info->parameters.fft_samples, audio_info->parameters.fft_duration_ms);

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
