#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "read_audio.h"
#include "process_audio.h"
// #include "fft.h"
#include "show_audio.h"
// #include "arduinoFFT/arduinoFFT.h"

//gcc -Wall flash.c read_audio.c process_audio.c fft.c -o flash -lm && ./flash
//gcc -Wall flash.c -Ilib -o flash && ./flash

// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/

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
		samples_chunk chunk = read_audio(fp, config.min_samples);
		if (chunk.length == 0) {
			free(chunk.samples);
			break;
		}

		// printf("\n\n");
		// printf("SAMPLES: ");
		draw_wave(chunk.samples, chunk.length);
		// print_wave_values(chunk.samples, chunk.length);

		// cplx buf[chunk.length];
		// for (i = 0; i < chunk.length; i++) {
		// 	buf[i] = chunk.samples[i];
		// }
		// fft(buf, chunk.length);
		//
		// printf("\n");
		// fft_show("FFT: ", buf);

		free(chunk.samples);
	}

	close_audio(fp);
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

