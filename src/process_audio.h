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
	int min_freq;
	int max_freq;
	int min_sample_rate;
	int min_samples;
	float min_samples_duration_ms;
} audio_config;

typedef struct {
	double min; // Frecuencia de inicio de la banda
	double center; // Frecuencia central de la banda (no es lineal)
	double max; // Frecuencia final de la banda
	double value; // Alto de la banda
} t_frequency_band;

typedef struct {
	t_frequency_band* values;
	int length;
} t_frequency_band_array;

typedef struct {
	double* real;
	double* imaginary;
	int length;
	double sample_rate;
} t_fft;

audio_config new_audio_config(int min_freq, int max_freq);
void signal_to_fft(t_fft* fft);
double bpf_sum(double f_min, double f_max, t_fft* fft);
double bpf_average(double f_min, double f_max, t_fft* fft);
double fft_bin_bandwidth(int fft_length, double fft_sample_rate);
double fft_index_to_frequency(int fft_index, int fft_length, double fft_sample_rate);
double frequency_to_fft_index(double frequency, int fft_length, double fft_sample_rate);
void init_bands(t_frequency_band_array* fb_array, double f_min, double f_max);
void fft_to_bands(t_fft* fft, t_frequency_band_array* fb_array);

#endif
