#include <math.h>
#include "process_audio.h"

audio_config new_audio_config(int min_freq, int max_freq)
{
	// Para detectar una onda de 20 Hz necesito una muestra de al menos (1/20) seg = 0.05 seg
	// Si la frecuencia de muestreo es de 44100 Hz, significa que cada muestra dura (1/44100) seg = 0.000022676 seg
	// Es decir que necesito ((1/20) / (1/44100)) seg = seg 2205 muestras para
	// detectar una onda de 20 Hz si las muestras van a 44100 Hz.

	// De forma general, la cantidad mínima de samples requeridos es:
	// min_samples = sample_rate / min_freq

	audio_config config;
	config.min_freq = min_freq;
	config.max_freq = max_freq;
	config.min_sample_rate = max_freq * 2;
	config.min_samples = config.min_sample_rate / config.min_freq;
	config.min_samples_duration_ms = 1000 / config.min_freq;

	return config;
}

void signal_to_fft(double *real, double *imag, uint16_t samples, double sampling_frequency)
{
	#if USE_ARDUINO_FFT_MODULE
		// Con arduinoFFT.cpp:
		arduinoFFT fftInstance = arduinoFFT(real, imag, samples, sampling_frequency);

		// Compute FFT
		fftInstance.DCRemoval();
		fftInstance.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
		fftInstance.Compute(FFT_FORWARD);
		fftInstance.ComplexToMagnitude();
	#else
		// Con fft.c:
		fft(real, imag, samples);
	#endif
}

double fft_bin_bandwidth(unsigned int fft_length, double fft_sample_rate)
{
	// Cada posición del array de FFT ("bin") corresponde a una banda de frecuencia.
	// Todas las posiciones son del mismo ancho (bandwidth).

	// Con N bins:
	// Bandwidth: bw = 1 (T * N) = (1 / T) * (1 / N) = sample_rate / N

	return fft_sample_rate / fft_length;
}

// Relación entre frecuencia e índice:
// https://forum.arduino.cc/t/fft-results/594243/10

// Significado de las frecuencias negativas:
// https://dsp.stackexchange.com/questions/431/what-is-the-physical-significance-of-negative-frequencies

// Para 0 <= i < N/2: f = i * bw
// Para N/2 <= i < N: f = (i - N) * bw

double fft_index_to_frequency(unsigned int fft_index, unsigned int fft_length, double fft_sample_rate)
{
	if (fft_index <= 0 || fft_length == 0 || fft_sample_rate <= 0 || fft_index >= fft_length) return 0;

	double bw = fft_bin_bandwidth(fft_length, fft_sample_rate);

	if (fft_index < fft_length / 2.0) {
		// Para 0 <= i < N/2: f = i * bw
		return fft_index * bw;
	}

	// Para N/2 <= i < N: f = (i - N) * bw
	return (int)(fft_index - fft_length) * bw;
}

// La frecuencia puede ser negativa (corresponde a la 2da parte del array de FFT).
// El index puede ser un valor no entero.
double frequency_to_fft_index(double frequency, unsigned int fft_length, double fft_sample_rate)
{
	if (fft_length == 0 || fft_sample_rate <= 0) return 0;

	double bw = fft_bin_bandwidth(fft_length, fft_sample_rate);
	double bw_half = bw / 2.0;

	if (frequency >= 0 && frequency < fft_length * bw_half) {
		// Para 0 <= i < N/2: f = i * bw
		// Para 0 <= f < N * bw/2: i = f / bw
		return frequency / bw;
	} else if (frequency >= -bw_half && frequency < 0) {
		// Para N/2 <= i < N: f = (i - N) * bw
		// Para -bw/2 <= f < 0: i = N + f / bw
		return fft_length + frequency / bw;
	}

	return 0;
}

// A partir de la FFT, calcula la suma promedio entre dos frecuencias [f_min; f_max)
// Si la frecuencia cae en un valor no entero de una posición de FFT, se suma ponderada,
// como si calculara el área de una función discreta, segmentada por cada posición.
double bpf_average(double f_min, double f_max, double *fft_real, unsigned int fft_length, double fft_sample_rate)
{
	double p_min = frequency_to_fft_index(f_min, fft_length, fft_sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft_length, fft_sample_rate);
	unsigned int i_min = ceil(p_min);
	unsigned int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	double sum = 0;

	if (i_min >= 1) {
		// Sumo parte fraccionaria del inicio, ponderada:
		sum += (i_min - p_min) * fft_real[i_min - 1];
	}

	if (i_max < fft_length) {
		// Sumo parte fraccionaria del final, ponderada:
		sum += (p_max - i_max) * fft_real[i_max];
	}

	// Sumo partes enteras:
	for (unsigned int i = i_min; i < i_max; i++) {
		sum += fft_real[i];
	}

	// Promedio:
	return sum / (p_max - p_min);
}

// El oído no percibe linealmente las frecuencias (por ejemplo, una octava es
// el doble de frecuencia).
// Por esto, una frecuencia de 100 Hz se parece a una de 200 Hz en igual medida
// que una de 10 KHz se parece a otra de 20 KHz.
