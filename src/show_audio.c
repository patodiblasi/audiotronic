#include <ncurses.h>
#include "show_audio.h"
#include "process_audio.h"

WINDOW* header_window;
WINDOW* body_window;

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
	for (int i = 0; i < length; i++) {
		printf("\n%d\t%d", i, (short int)samples[i]);
	}
}

int ncurses_start()
{
	printf("Iniciando modo NCURSES");

	initscr();
	curs_set(0);
	clear();
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

	header_window = newwin(10, 180, 0, 4);
	body_window = newwin(30, 180, 10, 4);

	box(header_window, 0 , 0);
	draw_logo(header_window);
	wrefresh(header_window);

	return 1;
}

void ncurses_end()
{
	delete_win(header_window);
	delete_win(body_window);
	endwin();
}

int ncurses_loop(t_fft* fft)
{
	// Esto es bloqueante, así que no me sirve!
	// char c = wgetch(header_window);
	// switch(c) {
	// 	case 27: // ESC
	// 		return 0;
	// }

	// TODO: sacar del loop
	int bands_length = 28;
	t_frequency_band bands_values[bands_length];
	t_frequency_band_array band_array;
	band_array.values = bands_values;
	band_array.length = bands_length;

	wclear(body_window);
	box(body_window, 0 , 0);
	draw_fft(body_window, fft, &band_array);
	wrefresh(body_window);

	return 1;
}

void delete_win(WINDOW* win)
{
	/* box(win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners
	 * and so an ugly remnant of window.
	 */
	wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window
	 * 3. rs: character to be used for the right side of the window
	 * 4. ts: character to be used for the top side of the window
	 * 5. bs: character to be used for the bottom side of the window
	 * 6. tl: character to be used for the top left corner of the window
	 * 7. tr: character to be used for the top right corner of the window
	 * 8. bl: character to be used for the bottom left corner of the window
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(win);
	delwin(win);
}

void draw_logo(WINDOW* win)
{
	wattron(win, COLOR_PAIR(COLOR_PAIR_PRIMARY));

	mvwprintw(win, 2, 4, "           _     __  ___  _   __          __ ");
	mvwprintw(win, 3, 4, " /\\  |  | | \\ | |  |  |  |_/ |  | |\\ | | |   ");
	mvwprintw(win, 4, 4, "/  \\ |__| |_/ | |__|  |  | \\ |__| | \\| | |__ ");
}

// bands: Cantidad de bandas en que se divide el resultado
void draw_fft(WINDOW* win, t_fft* fft, t_frequency_band_array* band_array)
{
	int win_height;
	int win_width;
	getmaxyx(win, win_height, win_width);

	int bar_bottom = win_height - 5;
	int bar_height = win_height - 10;
	int band_containter_width = 6;
	int margin_left = 5;

	fft_to_bands(fft, band_array);

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

		wattron(win, COLOR_PAIR(COLOR_PAIR_PRIMARY));
		for (int j = bar_bottom; j > y; j--) {
			mvwprintw(win, j, x, "   ");
		}

		if (is_cropped) {
			wattron(win, COLOR_PAIR(COLOR_PAIR_ERROR));
			mvwprintw(win, y, x, "   ");
			mvwprintw(win, bar_bottom+2, x, "%ld", lround(band->min));
			mvwprintw(win, bar_bottom+3, x, "Hz");

			mvwprintw(win, bar_bottom - bar_height - 2, x, "%.2f", band->value);
		} else {
			wattron(win, COLOR_PAIR(COLOR_PAIR_ACCENT));
			mvwprintw(win, y, x, "   ");
			wattron(win, COLOR_PAIR(COLOR_PAIR_TEXT));
			mvwprintw(win, bar_bottom+2, x, "%ld", lround(band->min));
			mvwprintw(win, bar_bottom+3, x, "Hz");

			mvwprintw(win, bar_bottom - bar_height - 2, x, "%.2f", band->value);
		}
	}
}
