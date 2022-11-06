#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "read_audio.h"
#include "process_audio.h"
#include "show_audio.h"
#include "fft.h"

//gcc -Wall flash.c read_audio.c process_audio.c fft.c -o flash -lm && ./flash
//gcc -Wall flash.c -Ilib -o flash && ./flash

// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/

#define INPUT_FILE "input/prueba2.wav"

int main(void)
{
	audio_config config = new_audio_config(4000, 22050);
	printf("Leyendo de a %d samples a %d Hz (%.2f ms)", config.min_samples, config.min_sample_rate, config.min_samples_duration_ms);

	////////////////////////////////////////////////////////////////////////////
	FILE* fp = open_audio(INPUT_FILE);
	if (!fp) {
		printf ("\nError abriendo audio.\n");
	}

	printf("\n\n");

	int i=0;

	while (1) {
		printf("\n-------------------------\nREAD AUDIO:\n");
		samples_chunk chunk = read_audio(fp, config.min_samples);
		if (chunk.length == 0) {
			printf("\n---------- END ----------\n");
			free(chunk.samples);
			break;
		}

		// printf("\n\n");
		// printf("\n-------------------------\nSAMPLES:\n");
		// printf("SAMPLES: ");
		// draw_wave(chunk.samples, chunk.length);
		// print_wave_values(chunk.samples, chunk.length);

		size_t fft_size = sizeof(float) * chunk.length;
		float* vReal = malloc(fft_size);
		float* vImag = malloc(fft_size);

		for (i = 0; i < chunk.length; i++) {
			vReal[i] = (float)(chunk.samples[i]);
			vImag[i] = 0;
		}

		rearrange(vReal, vImag, chunk.length);
		printf("\n-------------------------\nFFT (rearrange):\n");
		for (i = 0; i < chunk.length; i++) {
			printf("\n%d\t%d\t%.2f\t%.2f", i, chunk.samples[i], vReal[i], vImag[i]);
		}
		compute(vReal, vImag, chunk.length);
		printf("\n-------------------------\nFFT (compute):\n");
		for (i = 0; i < chunk.length; i++) {
			printf("\n%d\t%d\t%.2f\t%.2f", i, chunk.samples[i], vReal[i], vImag[i]);
		}

		// fft(vReal, vImag, chunk.length);
		// printf("\n-------------------------\nFFT:\n");
		// for (i = 0; i < chunk.length; i++) {
		// 	printf("\n%d\t%d\t%.2f\t%.2f", i, chunk.samples[i], vReal[i], vImag[i]);
		// }
		printf("\n\n\n");
		free(chunk.samples);
		free(vReal);
		free(vImag);
	}

	close_audio(fp);
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

