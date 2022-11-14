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

void signal_to_fft(t_fft* fft)
{
	#if USE_ARDUINO_FFT_MODULE
		// Con arduinoFFT.cpp:
		arduinoFFT fftInstance = arduinoFFT(fft->real, fft->imaginary, fft->length, fft->sample_rate);

		// Compute FFT
		// DCRemoval genera más ruido del que elimina... Overflow?
		// fftInstance.DCRemoval();
		fftInstance.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
		fftInstance.Compute(FFT_FORWARD);
		fftInstance.ComplexToMagnitude();
	#else
		// Con fft.c:
		fft(fft->real, fft->imaginary, fft->length);

		// Hago absolutos todos los valores:
		fft_amplitude_to_magnitude(fft->real, fft->length);
	#endif
}

double fft_bin_bandwidth(unsigned int fft_length, double fft_sample_rate)
{
	// Cada posición del array de FFT ("bin") corresponde a una banda de frecuencia.
	// Todas las posiciones son del mismo ancho (bandwidth).

	// Con N bins:
	// Bandwidth: bw = 1 (T * N) = (1 / T) * (1 / N) = sample_rate / N

	return fft_sample_rate / (double)fft_length;
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

	if (fft_index < (double)fft_length / 2.0) {
		// Para 0 <= i < N/2: f = i * bw
		return (double)fft_index * bw;
	}

	// Para N/2 <= i < N: f = (i - N) * bw
	return ((double)fft_index - (double)fft_length) * bw;
}

// La frecuencia puede ser negativa (corresponde a la 2da parte del array de FFT).
// El index puede ser un valor no entero.
double frequency_to_fft_index(double frequency, unsigned int fft_length, double fft_sample_rate)
{
	if (fft_length == 0 || fft_sample_rate <= 0) return 0;

	double bw = fft_bin_bandwidth(fft_length, fft_sample_rate);
	double bw_half = bw / 2.0;

	if (frequency >= 0 && frequency < (double)fft_length * bw_half) {
		// Para 0 <= i < N/2: f = i * bw
		// Para 0 <= f < N * bw/2: i = f / bw
		return frequency / bw;
	} else if (frequency >= -bw_half && frequency < 0) {
		// Para N/2 <= i < N: f = (i - N) * bw
		// Para -bw/2 <= f < 0: i = N + f / bw
		return (double)fft_length + frequency / bw;
	}

	return 0;
}

double bpf_sum(double f_min, double f_max, t_fft* fft)
{
	double p_min = frequency_to_fft_index(f_min, fft->length, fft->sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft->length, fft->sample_rate);
	unsigned int i_min = ceil(p_min);
	unsigned int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	if (p_max == p_min) return 0;

	double sum = 0;

	// Si p_max - p_min < 1 ==> i_min > i_max
	// Para no sumar 2 veces ese segmento, valido que no se crucen
	if (i_min < i_max) {
		if (i_min >= 1) {
			// Sumo parte fraccionaria del inicio, ponderada:
			sum += ((double)i_min - p_min) * fft->real[i_min - 1];
		}

		if (i_max < fft->length) {
			// Sumo parte fraccionaria del final, ponderada:
			sum += (p_max - (double)i_max) * fft->real[i_max];
		}

		// Sumo partes enteras:
		for (unsigned int i = i_min; i < i_max; i++) {
			sum += fft->real[i];
		}
	} else if (i_max < fft->length) {
		// p_max - p_min < 1
		// Sumo un solo elemento, sacando los márgenes:
		sum += ((double)p_max - p_min) * fft->real[i_max];
	}

	return sum;
}

// A partir de la FFT, calcula la suma promedio entre dos frecuencias [f_min; f_max)
// Si la frecuencia cae en un valor no entero de una posición de FFT, se suma ponderada,
// como si calculara el área de una función discreta, segmentada por cada posición.
double bpf_average(double f_min, double f_max, t_fft* fft)
{
	double p_min = frequency_to_fft_index(f_min, fft->length, fft->sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft->length, fft->sample_rate);
	unsigned int i_min = ceil(p_min);
	unsigned int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	if (p_max == p_min) return 0;

	double sum = bpf_sum(f_min, f_max, fft);

	// Promedio:
	return sum / (p_max - p_min);
}

// Calcula las frecuencias en donde empiezan las bandas, de forma tal que la
// relación musical entre las mismas sea igual de una banda a la siguiente.
void init_bands(t_frequency_band_array* fb_array, double f_min, double f_max)
{
	// El oído no percibe linealmente las frecuencias (por ejemplo, una octava es
	// el doble de frecuencia).
	// Por esto, una frecuencia de 100 Hz se parece a una de 200 Hz en igual medida
	// que una de 10 KHz se parece a otra de 20 KHz.

	// Ver:
	// https://en.wikipedia.org/wiki/Equal_temperament
	// http://people.cs.uchicago.edu/~odonnell/Scholar/Work_in_progress/Digital_Sound_Modelling/lectnotes/node4.html
	// https://stackoverflow.com/questions/19472747/convert-linear-scale-to-logarithmic

	// Quiero un array con las frecuencias en donde empiezan las bandas, de
	// forma tal que el próximo índice sería f_max.

	// (x - x0) / (x1 - x0) = (log(y) - log(y0)) / (log(y1) - log(y0))
	// log(y) = log(y0) + (x - x0) * (log(y1) - log(y0)) / (x1 - x0)
	// y = 2 ^ ( log(y0) + (x - x0) * (log(y1) - log(y0)) / (x1 - x0) )

	// x0: 0, x1: fb_array->length, y0: f_min, y1: f_min
	double log2_y0 = f_min == 0 ? 0 : log2(f_min);
	double d = (log2(f_max) - log2_y0) / (double)(fb_array->length);

	fb_array->values[0].min = f_min;
	for (int x = 1; x < fb_array->length; x++) {
		double f = pow(2, log2_y0 + (double)x * d);
		fb_array->values[x-1].max = f;
		fb_array->values[x].min = f;

		// El centro de la banda, en escala logarítmica, es el mismo cálculo,
		// pero como si tuviera 2 bandas, con x0 = 0, x1 = 2, evaluado en x = 1.

		// y = 2 ^ ( log(y0) + (x - x0) * (log(y1) - log(y0)) / (x1 - x0) )
		// Después de simplificar, queda:
		// y = raíz (y0 * y1)
		fb_array->values[x-1].center = sqrt(fb_array->values[x-1].max * fb_array->values[x-1].min);
	}
	fb_array->values[fb_array->length-1].max = f_max;
}

// Los valores de las bandas son normalizados de 0 a 1, pero pueden existir
// valores fuera de escala (no se verifica límite).
void fft_to_bands(t_fft* fft, t_frequency_band_array* fb_array)
{
	// Con 24 llega justo al tope, pero los agudos son muy débiles... Revisar
	double max_amplitude = pow(2, 20) - 1;

	init_bands(fb_array, 20, 20000);

	for (int i = 0; i < fb_array->length; i++) {
		int is_cropped = 0;
		fb_array->values[i].value = bpf_average(
			fb_array->values[i].min,
			fb_array->values[i].max,
			fft
		) / max_amplitude;
	}
}

