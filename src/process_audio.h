#ifndef PROCESS_AUDIO_INCLUDED
#define PROCESS_AUDIO_INCLUDED

#define FFT_MODULE_ARDUINO 1 // Si es true, usa el módulo de Arduino para FFT

#include <stdint.h>

#if FFT_MODULE_ARDUINO
	#include "arduinoFFT/arduinoFFT.h"
#else
	#include "fft.h"
#endif

typedef struct {
	unsigned int min_freq;
	unsigned int max_freq;
	unsigned int min_sample_rate;
	unsigned int min_samples;
	float min_samples_duration_ms;
} audio_config;

audio_config new_audio_config(int min_freq, int max_freq);
void signal_to_fft(double *real, double *imag, uint16_t samples, double sampling_frequency);

#endif
