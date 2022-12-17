#include "test.h"
#include "main_audio.h"
#include <math.h>
/*
void test_bpf_average(t_audio_info* audio_info)
{
	// Test bpf_average:
	double max_avg = 0;
	int max_f = 0;
	for (int f = 0; f < 20000;) {
		int min = f;
		int max = f + 1000;

		double avg = bpf_average(min, max, audio_info->fft.real, audio_info->chunk.length, (double)audio_info->parameters.min_sample_rate);

		if (avg > max_avg) {
			max_avg = avg;
			max_f = f;
		}

		f = max;
	}

	printf("%d\n", max_f);
}

void test_fft(t_audio_info* audio_info)
{
	// Test FFT:
	double max_value = 0;
	int max_index = 0;
	// Es importante no tomar en cuenta la 2da mitad de la tabla de la FFT,
	// ya que contiene frecuencias negativas.
	for (int i = 1; i < audio_info->chunk.length / 2; i++) {
		if (audio_info->fft.real[i] > max_value) {
			max_value = audio_info->fft.real[i];
			max_index = i;
		}
	}
	double max_freq = fft_index_to_frequency(max_index, audio_info->chunk.length, audio_info->parameters.min_sample_rate);
	printf("%d\t%f\n", max_index, max_freq);
}
*/

// time, amplitude, frequency, phase
double f_sine(double t, double a, double f, double p) {
	return a * sin(2*M_PI*f*t + p);
}

void test_fft_2()
{
	int fft_length = 128;
	double sample_rate = 1760;
	double wave[fft_length];
	int i;
	double t;
	double amplitude = 1000;
	double freq = 442;

	t_fft fft;
	double fft_real[fft_length];
	double fft_imaginary[fft_length];
	fft.real = fft_real;
	fft.imaginary = fft_imaginary;
	fft.length = fft_length;
	fft.sample_rate = sample_rate;

	for (i = 0; i < fft_length; i++) { // create samples
		t = i * 1 / sample_rate;
		wave[i] = f_sine(t, amplitude, freq, 0);
		fft.real[i] = wave[i];
	}

	signal_to_fft(&fft);

	printf("bin\t\t\twave\t\t\tfreq\t\t\tamplitude\n");
	for (i=0; i<fft.length/2; i++) {
		printf("%d\t\t\t%.2f\t\t\t%.2f\t\t\t%.2f\n", i, wave[i], fft_index_to_frequency(i, fft.length, fft.sample_rate), fft.real[i]); //*2 for "negative frequency" amplitude
	}
}
