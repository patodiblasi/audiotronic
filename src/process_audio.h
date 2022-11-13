#ifndef IS_PROCESS_AUDIO_INCLUDED
#define IS_PROCESS_AUDIO_INCLUDED

#ifndef USE_ARDUINO_FFT_MODULE
	#define USE_ARDUINO_FFT_MODULE 1 // Si es true, usa el módulo de Arduino para FFT
#endif

#include <stdint.h>

#if USE_ARDUINO_FFT_MODULE
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
double bpf_average(double f_min, double f_max, double *fft_real, unsigned int fft_length, double fft_sample_rate);
double fft_bin_bandwidth(unsigned int fft_length, double fft_sample_rate);
double fft_index_to_frequency(unsigned int fft_index, unsigned int fft_length, double fft_sample_rate);
double frequency_to_fft_index(double frequency, unsigned int fft_length, double fft_sample_rate);

#endif
