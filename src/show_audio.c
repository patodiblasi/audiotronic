#include <ncurses.h>
#include "show_audio.h"
#include "process_audio.h"

// samples_per_line: Indica qué tanto se comprime verticalmente el dibujo.
// La cantidad de samples indicados por samples_per_line se promedian para formar una sola línea.
// multiplier: Multiplicador para la amplitud. Si el resultado excede el máximo, se cropea.
void draw_wave(int16_t* samples, unsigned int length, unsigned int samples_per_line, double multiplier)
{
	unsigned int max_amplitude = 65535;

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

void print_wave_values(int16_t* samples, unsigned int length)
{
	for (int i = 0; i < length; i++) {
		printf("\n%d\t%d", i, (short int)samples[i]);
	}
}

void screen_start()
{
	initscr();
	noecho();

	// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/can_change_color.html
	start_color();

	attrset(0);
	if (can_change_color()) {
		// init_color recibe valores de 0 a 1000:
		init_color(COLOR_BLACK, 0, 0, 0);
		init_color(COLOR_WHITE, 1000, 1000, 1000);
		init_color(COLOR_BLUE, 1000*0x38/0xff, 1000*0x46/0xff, 1000*0xc0/0xff);
		init_color(COLOR_RED, 1000*0xc6/0xff, 1000*28/0xff, 1000*28/0xff);
	}

	init_pair(COLOR_PAIR_TEXT, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_PAIR_ACCENT, COLOR_BLACK, COLOR_WHITE);
	init_pair(COLOR_PAIR_PRIMARY, COLOR_WHITE, COLOR_BLUE);
	init_pair(COLOR_PAIR_ERROR, COLOR_WHITE, COLOR_RED);

	draw_logo();
}

void screen_end()
{
	endwin();
}

void draw_logo()
{
	attron(COLOR_PAIR(COLOR_PAIR_PRIMARY));

	mvprintw(1, 5, "           _     __  ___  _   __          __ ");
	mvprintw(2, 5, " /\\  |  | | \\ | |  |  |  |_/ |  | |\\ | | |   ");
	mvprintw(3, 5, "/  \\ |__| |_/ | |__|  |  | \\ |__| | \\| | |__ ");
}

// bands: Cantidad de bandas en que se divide el resultado
void screen_draw_fft(t_fft* fft, t_frequency_band_array* band_array)
{
	int bar_bottom = 45;
	int bar_height = 30;
	int band_containter_width = 6;
	int margin_left = 5;

	// Con 24 llega justo al tope, pero los agudos son muy débiles... Revisar
	double max_amplitude = pow(2, 24) - 1;

	fft_to_bands(fft, band_array);

	// getmaxyx(stdscr, row, col);
	clear();
	for (int i = 1; i <= band_array->length; i++) {
		int is_cropped = 0;
		int i0 = i-1;
		double value_cropped;
		t_frequency_band* band = &(band_array->values[i]);
		if (band->value > 1) {
			is_cropped = 1;
			value_cropped = 1;
		} else if (band->value < 0) {
			is_cropped = 1;
			value_cropped = 0;
		} else {
			value_cropped = band->value;
		}

		int y = bar_bottom - bar_height * value_cropped;
		int x = margin_left + i0 * band_containter_width;

		attron(COLOR_PAIR(COLOR_PAIR_PRIMARY));
		for (int j = bar_bottom; j > y; j--) {
			mvprintw(j, x, "   ");
		}
		draw_logo();
		if (is_cropped) {
			attron(COLOR_PAIR(COLOR_PAIR_ERROR));
			mvprintw(y, x, "   ");
			mvprintw(bar_bottom+2, x, "%ld", lround(band->min));
			mvprintw(bar_bottom+3, x, "Hz");

			mvprintw(bar_bottom - bar_height - 2, x, "%.2f", band->value);
		} else {
			attron(COLOR_PAIR(COLOR_PAIR_ACCENT));
			mvprintw(y, x, "   ");
			attron(COLOR_PAIR(COLOR_PAIR_TEXT));
			mvprintw(bar_bottom+2, x, "%ld", lround(band->min));
			mvprintw(bar_bottom+3, x, "Hz");
		}
	}
	refresh();
}
