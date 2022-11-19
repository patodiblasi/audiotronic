// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/
// http://trac.ffmpeg.org/wiki/StreamingGuide
// https://dsp.stackexchange.com/questions/8317/fft-amplitude-or-magnitude
// http://dranger.com/ffmpeg/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "main_audio.h"
#include "screen.h"
#include "requests.h"
#include "envs.h"
#include "log/src/log.h"

struct timespec start_time = { -1, 0 }; // tv_sec, tv_nsec
t_audio_info audio_info;
t_screen_data screen_data;

////////////////////////////////////////////////////////////////////////////////

void print_separator()
{
	printf("\n====================================================================================================\n");
	fflush(stdout);
	fflush(stderr);
}

// Retorna tiempo en microsegundos desde algún punto desconocido
long int get_utime()
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

void close()
{
	screen_end();
	audio_end(&audio_info);
	print_separator();
	exit(0);
}

void on_exit_signal(int s)
{
	log_warn("Se recibió señal de salida %d.", s);
	close();
}

////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	signal(SIGINT, on_exit_signal);
	signal(SIGTSTP, on_exit_signal);
	signal(SIGTERM, on_exit_signal);

	print_separator();

	screen_set_mode(SCREEN_MODE_NCURSES);
	if (!screen_start()) {
		close();
	}

	if (!check_envs()) {
		// No corto la ejecución
		log_error("No fue posible cargar todas las variables de entorno.");
	}

	init_requests();
	turn_off();

	if (!audio_setup(&audio_info)) {
		close();
	}

	////////////////////////////////////////////////////////////////////////////

	// Limitadores de ciclos
	// Todo en microsegundos
	long now;
	long last_audio_time = 0;
	long last_video_time = 0;
	long audio_frame_duration = 1000 * audio_info.config.min_fft_duration_ms; // Aprox 1/20 = 50 ms
	long video_frame_duration = (1000000 / 30); // 1/60 = 16,66 ms

	int run_audio_frame = 1;
	int run_video_frame = 1;
	int continue_loop = 1;

	long frame_number = 0;

	while (continue_loop) {
		////////////////////////////////////////////////////////////////////////
		// Control de frames de audio / video:

		frame_number++;
		now = get_utime();
		run_audio_frame = (now - last_audio_time) >= audio_frame_duration;
		run_video_frame = (now - last_video_time) >= video_frame_duration;

		if (run_audio_frame) {
			last_audio_time = now;
		}
		if (run_video_frame) {
			last_video_time = now;
		}
		////////////////////////////////////////////////////////////////////////
		if (run_audio_frame && continue_loop) {
			continue_loop = audio_loop(&audio_info);
		}

		if (run_video_frame && continue_loop) {
			screen_data.audio_in = &audio_info.audio_in;
			screen_data.wave = &audio_info.chunk;
			screen_data.fft = &audio_info.fft;
			continue_loop = screen_loop(&screen_data);
		}

		if (frame_number % 10 == 0) {
			turn_on();
		} else {
			turn_off();
		}
	}

	close();
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

