// http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
// https://rosettacode.org/wiki/Fast_Fourier_transform#C
// https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/
// http://trac.ffmpeg.org/wiki/StreamingGuide

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "main_audio.h"
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

////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	printf("\n\n");

	t_audio_info audio_info;
	audio_setup(&audio_info);

	// sdl_screen screen = start_screen(800, 600);
	// if (!screen.is_ok) {
	// 	fprintf(stderr, "\nError creando pantalla.\n");
	// 	close_screen(screen);
	// 	return 2;
	// }

	////////////////////////////////////////////////////////////////////////////

	// Limitadores de ciclos
	// Todo en microsegundos
	unsigned long now;
	unsigned long last_audio_time = 0;
	unsigned long last_video_time = 0;
	unsigned long audio_frame_duration = 1000 * audio_info.config.min_samples_duration_ms; // Aprox 1/20 = 50 ms
	unsigned long video_frame_duration = (1000000 / 60); // 1/60 = 16,66 ms

	int run_audio_frame = 1;
	int run_video_frame = 1;
	int continue_loop = 1;

	unsigned long frame_number = 0;

	while (continue_loop) {
		////////////////////////////////////////////////////////////////////////
		// Control de frames de audio / video:

		frame_number++;
		now = get_utime();
		run_audio_frame = (now - last_audio_time) >= audio_frame_duration;
		run_video_frame = (now - last_video_time) >= video_frame_duration;
		if (run_audio_frame) {
			// printf("\nframe %ld: %ld AUDIO", frame_number, now);
			last_audio_time = now;
		}
		if (run_video_frame) {
			// printf("\nframe %ld: %ld VIDEO", frame_number, now);
			last_video_time = now;
		}
		////////////////////////////////////////////////////////////////////////

		if (run_audio_frame) {
			continue_loop = continue_loop && audio_loop_start(&audio_info);
		}

		if (run_video_frame) {
			// continue_loop = continue_loop && screen_frame(screen, audio_info.chunk.samples, audio_info.chunk.length, audio_info.real, audio_info.chunk.length);
		}

		// OJO: Tiene que ir después del frame de video
		if (run_audio_frame) {
			continue_loop = continue_loop && audio_loop_end(&audio_info);
		}
	}

	audio_end(&audio_info);
	// close_screen(screen);
	////////////////////////////////////////////////////////////////////////////

	return 0;
}

