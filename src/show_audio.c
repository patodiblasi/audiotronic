#include "show_audio.h"

void draw_wave(int16_t* samples, unsigned int length)
{
	int i = 0;
	int j = 0;
	int r = 0;

	// Qué tanto se comprime gráficamente el dibujo.
	// La cantidad de samples indicados por reduce se promedian para formar un solo valor
	int reduce = 200;

	int scale = 200;
	int limit = scale / 2;

	for (i = 0; i < length; i += reduce) {
		int sum_to_reduce = 0;
		int sum_count = reduce;
		if (i + sum_count > length) {
			sum_count = length - i;
		}

		for (r = 0; r < sum_count; r++) {
			sum_to_reduce += limit * samples[i + r] / (double)65535;
		}
		// printf("\n%d %d", sum_to_reduce, sum_count);
		// TODO: optimizar con shift?
		int amplitude_percentage = sum_to_reduce / sum_count;
		printf("\n");
		if (amplitude_percentage < 0) {
			for (j = -limit; j < amplitude_percentage; j++) {
				printf(" ");
			}
			for (j = amplitude_percentage; j < 0; j++) {
				printf("-");
			}
		} else {
			for (j = -limit; j < 0; j++) {
				printf(" ");
			}
			for (j = 0; j < amplitude_percentage; j++) {
				printf("-");
			}
		}
	}
}

void print_wave_values(int16_t* samples, unsigned int length)
{
	for (int i = 0; i < length; i++) {
		printf("\n%d\t%d", i, (short int)samples[i]);
	}
}
