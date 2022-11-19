#include "main_audio.h"

void test_bpf_average(t_audio_info* audio_info)
{
	// Test bpf_average:
	double max_avg = 0;
	int max_f = 0;
	for (int f = 0; f < 20000;) {
		int min = f;
		int max = f + 1000;

		double avg = bpf_average(min, max, audio_info->fft.real, audio_info->chunk.length, (double)audio_info->config.min_sample_rate);

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
	double max_freq = fft_index_to_frequency(max_index, audio_info->chunk.length, audio_info->config.min_sample_rate);
	printf("%d\t%f\n", max_index, max_freq);
}
