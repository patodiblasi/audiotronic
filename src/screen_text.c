#include "screen_text.h"
#include <stdlib.h>
#include <stdio.h>
#include "log/src/log.h"

int screen_text_start()
{
	log_set_use_color(true);
	log_info("Iniciando modo TEXTO");
	return 1;
}

int screen_text_end()
{
	log_info("Terminando modo TEXTO");
	return 1;
}

int screen_text_loop(t_screen_data* data)
{
	log_debug("Loop modo TEXTO");
	// print_wave_values(data->wave->samples, data->wave->length);
	return 1;
}

// samples_per_line: Indica qué tanto se comprime verticalmente el dibujo.
// La cantidad de samples indicados por samples_per_line se promedian para formar una sola línea.
// multiplier: Multiplicador para la amplitud. Si el resultado excede el máximo, se cropea.
void draw_wave(int16_t* samples, int length, int samples_per_line, double multiplier)
{
	int max_amplitude = 65535;

	int i = 0;
	int j = 0;
	int r = 0;

	int scale = 200;
	int col_limit = scale / 2;

	for (i = 0; i < length; i += samples_per_line) {
		int line_sum = 0;
		int sum_count = samples_per_line;
		if (i + sum_count > length) {
			sum_count = length - i;
		}

		for (r = 0; r < sum_count; r++) {
			line_sum += samples[i + r];
		}

		// TODO: optimizar con shift?
		int amplitude_columns = col_limit * multiplier * line_sum / (double)(sum_count * max_amplitude);

		if (amplitude_columns > col_limit) {
			amplitude_columns = col_limit;
		} else if (amplitude_columns < -col_limit) {
			amplitude_columns = -col_limit;
		}

		printf("\n");
		if (amplitude_columns < 0) {
			for (j = -col_limit; j < amplitude_columns; j++) {
				printf(" ");
			}
			for (j = amplitude_columns; j < 0; j++) {
				printf("-");
			}
		} else {
			for (j = -col_limit; j < 0; j++) {
				printf(" ");
			}
			for (j = 0; j < amplitude_columns; j++) {
				printf("-");
			}
		}
	}
}

void print_wave_values(int16_t* samples, int length)
{
	if (length == 0) {
		printf("0 samples\n");
		return;
	}
	for (int i = 0; i < length; i++) {
		printf("%d\t%d\n", i, (short int)samples[i]);
	}
}
