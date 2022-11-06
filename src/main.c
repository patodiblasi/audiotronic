// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "read_audio.h"
#include "process_audio.h"
#include "show_audio.h"

#define INPUT_FILE "input/prueba2.wav"

int main(void)
{
	audio_config config = new_audio_config(20, 22050);
	printf("Leyendo de a %d samples a %d Hz (%.2f ms)", config.min_samples, config.min_sample_rate, config.min_samples_duration_ms);

	////////////////////////////////////////////////////////////////////////////
	FILE* fp = open_audio(INPUT_FILE);
	if (!fp) {
		printf ("\nError abriendo audio.\n");
	}

	printf("\n\n");

	while (1) {
		// printf("\n-------------------------\nREAD AUDIO:\n");
		samples_chunk chunk = read_audio(fp, config.min_samples);
		if (chunk.length == 0) {
			// printf("\n---------- END ----------\n");
			free(chunk.samples);
			break;
		}

		// printf("\n\n");
		// printf("\n-------------------------\nSAMPLES:\n");
		// printf("SAMPLES: ");
		// draw_wave(chunk.samples, chunk.length);
		// print_wave_values(chunk.samples, chunk.length);

		// Copio la señal a dos nuevos arrays que van a necesitar las funciones de FFT
		size_t fft_size = sizeof(double) * chunk.length;
		double* real = (double*)malloc(fft_size);
		double* imag = (double*)malloc(fft_size);

		for (unsigned int i = 0; i < chunk.length; i++) {
			real[i] = (double)(chunk.samples[i]);
			imag[i] = 0;
		}

		signal_to_fft(real, imag, chunk.length, config.min_sample_rate);

		////////////////////////////////////////////////////////////////////////
		printf("\n-------------------------\nFFT:\n");
		for (unsigned int i = 0; i < chunk.length; i++) {
			printf("\n%d\t%d\t%.2f\t%.2f", i, chunk.samples[i], real[i], imag[i]);
			// printf("\n%d\t%d", i, chunk.samples[i]);
		}
		printf("\n\n\n");
		free(chunk.samples);
		free(real);
		free(imag);
	}

	close_audio(fp);
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

