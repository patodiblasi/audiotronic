// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/
// http://trac.ffmpeg.org/wiki/StreamingGuide

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "read_audio.h"
#include "process_audio.h"
#include "show_audio.h"
#include "sdl_screen.h"

struct timespec start_time = { -1, 0 }; // tv_sec, tv_nsec

// Retorna tiempo en microsegundos desde algún punto desconocido
long unsigned int get_utime()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);

	if (start_time.tv_sec == -1) {
		// Inicializo
		start_time = now;
	}

	// mili: 10^3
	// micro: 10^6
	// nano: 10^9

	return (now.tv_sec  - start_time.tv_sec) * 1000000 + (now.tv_nsec - start_time.tv_nsec) / 1000;
}

int main(void)
{
	int empty_stream_count = 0;
	audio_config config = new_audio_config(20, 22050);
	printf("Leyendo de a %d samples a %d Hz (%.2f ms)", config.min_samples, config.min_sample_rate, config.min_samples_duration_ms);

	sdl_screen screen = start_screen(800, 600);
	if (!screen.is_ok) {
		fprintf(stderr, "\nError creando pantalla.\n");
		close_screen(screen);
		return 2;
	}

	////////////////////////////////////////////////////////////////////////////
	FILE* fp = open_audio_file("audios/sentinel.wav");
	// FILE* fp = open_audio_device("alsa", "front:CARD=USB,DEV=0", config.min_sample_rate);
	if (!fp) {
		fprintf(stderr, "\nError abriendo audio.\n");
	}

	printf("\n\n");

	samples_chunk chunk;
	double* real;
	double* imag;

	// Limitadores de ciclos
	// Todo en microsegundos
	unsigned long now;
	unsigned long last_audio_time = 0;
	unsigned long last_video_time = 0;
	unsigned long audio_frame_duration = config.min_samples_duration_ms * 1000;
	unsigned long video_frame_duration = 1000000 / 30; // 30 fps

	int run_audio_frame = true;
	int run_video_frame = true;
	int continue_loop = true;

	while (continue_loop) {
		// Control de frames de audio / video:
		now = get_utime();
		run_audio_frame = (now - last_audio_time) >= audio_frame_duration;
		run_video_frame = (now - last_video_time) >= video_frame_duration;
		if (run_audio_frame) {
			last_audio_time = now;
		}
		if (run_video_frame) {
			last_video_time = now;
		}

		if (run_audio_frame) {
			// printf("\n-------------------------\nREAD AUDIO:\n");
			chunk = read_audio(fp, config.min_samples);
			if (chunk.length == 0) {
				empty_stream_count++;
				if (empty_stream_count > 100) {
					printf("\n---------- FIN DE STREAM ----------\n");
					free(chunk.samples);
					break;
				}
			}

			empty_stream_count = 0;

			// draw_wave(chunk.samples, chunk.length);
			// print_wave_values(chunk.samples, chunk.length);

			// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
			size_t fft_size = sizeof(double) * chunk.length;
			real = (double*)malloc(fft_size);
			imag = (double*)malloc(fft_size);

			for (unsigned int i = 0; i < chunk.length; i++) {
				real[i] = (double)(chunk.samples[i]);
				imag[i] = 0;
			}

			signal_to_fft(real, imag, chunk.length, config.min_sample_rate);

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
		}

		if (run_video_frame) {
			continue_loop = screen_frame(screen, chunk.samples, chunk.length, real, chunk.length);
		}

		if (run_audio_frame) {
			free(chunk.samples);
			free(real);
			free(imag);
		}
	}

	close_audio(fp);
	close_screen(screen);
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

