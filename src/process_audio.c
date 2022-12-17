#include <math.h>
#include <stdio.h>
#include "process_audio.h"

// Debería ser 16, pero lo bajo para que sea más sensible
#define FFT_MAGNITUDE_MAX_DIGITS 10

int ceil_power_of_2(int x)
{
	// Posiblemente el compilador optimice log2, por lo que es mejor calcular
	// la potencia así, que usando operadores binarios.
	int pow = (int)log2(x);
	return 1 << (pow + 1);
}

t_audio_parameters new_audio_parameters(int min_freq, int max_freq)
{
	// Para detectar una onda de 20 Hz necesito una muestra de al menos su duración.
	// Una onda de 20 Hz dura: (1/20) seg = 0.05 seg
	// Si la frecuencia de muestreo es de 44100 Hz, significa que cada muestra dura (1/44100) seg = 0.000022676 seg
	// Es decir que necesito ((1/20) / (1/44100)) seg = seg 2205 muestras para
	// detectar una onda de 20 Hz, si las muestras van a 44100 Hz.

	// De forma general, la cantidad mínima de samples requeridos es:
	// min_fft_samples = sample_rate / min_freq

	t_audio_parameters parameters;
	parameters.min_freq = min_freq;
	parameters.max_freq = max_freq;
	parameters.min_sample_rate = max_freq * 2;
	parameters.min_fft_samples = parameters.min_sample_rate / parameters.min_freq;
	parameters.min_fft_duration_ms = 1000.0 / parameters.min_freq;
	parameters.fft_samples = ceil_power_of_2(parameters.min_fft_samples);
	parameters.fft_duration_ms = (parameters.fft_samples / parameters.min_freq) / 1000.0;

	return parameters;
}

void signal_to_fft(t_fft* fft)
{
	#if USE_ARDUINO_FFT_MODULE
		// Con arduinoFFT.cpp:
		arduinoFFT fftInstance = arduinoFFT(fft->real, fft->imaginary, (uint16_t)fft->length, fft->sample_rate);

		// Compute FFT
		// DCRemoval genera más ruido del que elimina... Overflow?
		// fftInstance.DCRemoval();
		// fftInstance.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
		fftInstance.Compute(FFT_FORWARD);
		fftInstance.ComplexToMagnitude();
	#else
		// Con fft.c:
		fft_compute(fft->real, fft->imaginary, (unsigned int)fft->length);
		fft_amplitude_to_magnitude(fft->real, fft->imaginary, fft->length);
	#endif

	adjust_fft_scale(fft);
}

// Setea la escala en que se expresan los valores de FFT en la misma escala de la onda de origen
void adjust_fft_scale(t_fft* fft)
{
	for (int i=0; i < fft->length; i++) {
		fft->real[i] = 2.0 * fft->real[i] / (double)fft->length;
	}
}

double fft_bin_bandwidth(int fft_length, double fft_sample_rate)
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

double fft_index_to_frequency(int fft_index, int fft_length, double fft_sample_rate)
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
double frequency_to_fft_index(double frequency, int fft_length, double fft_sample_rate)
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
	int i_min = ceil(p_min);
	int i_max = floor(p_max);

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
		for (int i = i_min; i < i_max; i++) {
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
	int i_min = ceil(p_min);
	int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	if (p_max == p_min) return 0;

	double sum = bpf_sum(f_min, f_max, fft);

	// Promedio:
	return sum / (p_max - p_min);
}

// Suma los bins entre 2 frecuencias (de manera fraccionaria si no son enteros),
// elevando al cuadrado cada uno de los valores
double bpf_sum_pow(double f_min, double f_max, t_fft* fft)
{
	double p_min = frequency_to_fft_index(f_min, fft->length, fft->sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft->length, fft->sample_rate);
	int i_min = ceil(p_min);
	int i_max = floor(p_max);

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
			sum += pow(((double)i_min - p_min) * fft->real[i_min - 1], 2);
		}

		if (i_max < fft->length) {
			// Sumo parte fraccionaria del final, ponderada:
			sum += pow((p_max - (double)i_max) * fft->real[i_max], 2);
		}

		// Sumo partes enteras:
		for (int i = i_min; i < i_max; i++) {
			sum += pow(fft->real[i], 2);
		}
	} else if (i_max < fft->length) {
		// p_max - p_min < 1
		// Sumo un solo elemento, sacando los márgenes:
		sum += pow(((double)p_max - p_min) * fft->real[i_max], 2);
	}

	return sum;
}

double bpf_rms(double f_min, double f_max, t_fft* fft)
{
	double p_min = frequency_to_fft_index(f_min, fft->length, fft->sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft->length, fft->sample_rate);
	int i_min = ceil(p_min);
	int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	if (p_max == p_min) return 0;

	double sum = bpf_sum_pow(f_min, f_max, fft);

	// https://es.wikipedia.org/wiki/Media_cuadr%C3%A1tica
	// RMS:
	return sqrt(sum / (p_max - p_min));
}


double bpf_max(double f_min, double f_max, t_fft* fft)
{
	double p_min = frequency_to_fft_index(f_min, fft->length, fft->sample_rate);
	double p_max = frequency_to_fft_index(f_max, fft->length, fft->sample_rate);
	int i_min = ceil(p_min);
	int i_max = floor(p_max);

	// Para prevenir imprecisiones por redondeo:
	if (p_min > i_min) p_min = i_min;
	if (p_max < i_max) p_max = i_max;

	if (p_max == p_min) return 0;

	double x = 0;
	double max = 0;
	double sum = 0;

	// Si p_max - p_min < 1 ==> i_min > i_max
	// Para no sumar 2 veces ese segmento, valido que no se crucen
	if (i_min < i_max) {
		if (i_min >= 1) {
			// Sumo parte fraccionaria del inicio, ponderada:
			x = ((double)i_min - p_min) * fft->real[i_min - 1];
			if (x > max) {
				max = x;
			}
			sum += x;
		}

		if (i_max < fft->length) {
			// Sumo parte fraccionaria del final, ponderada:
			x = (p_max - (double)i_max) * fft->real[i_max];
			if (x > max) {
				max = x;
			}
			sum += x;
		}

		// Sumo partes enteras:
		for (int i = i_min; i < i_max; i++) {
			x = fft->real[i];
			if (x > max) {
				max = x;
			}
			sum += x;
		}
	} else if (i_max < fft->length) {
		// p_max - p_min < 1
		// Sumo un solo elemento, sacando los márgenes:
		x = ((double)p_max - p_min) * fft->real[i_max];
		if (x > max) {
			max = x;
		}
		sum += x;
	}

	// Probando con sweep y mic llegué a estos valores fruta para la ponderación:
	return max * 0.97 + sum * 0.03;
}

// Calcula las frecuencias en donde empiezan las bandas, de forma tal que la
// relación musical entre las mismas sea igual de una banda a la siguiente.
void init_bands_log(t_frequency_band_array* fb_array, double f_min, double f_max)
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
	// ...
	// Simplificación Chapu:
	// y = y0 * (y1 / y0) ^ ((x - x0) / (x1 - x0))

	// x0: 0, x1: fb_array->length, y0: f_min, y1: f_max
	double f_proportion = f_max / f_min;

	fb_array->values[0].min = f_min;
	for (int x = 1; x < fb_array->length; x++) {
		double f = f_min * pow(f_proportion, x / (double)fb_array->length);
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

void init_bands_linear(t_frequency_band_array* fb_array, double f_min, double f_max)
{
	double band_width = (f_max - f_min) / fb_array->length;

	fb_array->values[0].min = f_min;
	for (int x = 1; x < fb_array->length; x++) {
		double f = x * band_width + f_min;
		fb_array->values[x-1].max = f;
		fb_array->values[x].min = f;

		fb_array->values[x-1].center = (fb_array->values[x-1].max + fb_array->values[x-1].min) / 2;
	}
	fb_array->values[fb_array->length-1].max = f_max;
}

// Los valores de las bandas son normalizados de 0 a 1, pero pueden existir
// valores fuera de escala (no se verifica límite).
void fft_to_bands_log(t_fft* fft, t_frequency_band_array* fb_array)
{
	double max_amplitude = (1 << FFT_MAGNITUDE_MAX_DIGITS) - 1;

	init_bands_log(fb_array, 20, 20000);

	for (int i = 0; i < fb_array->length; i++) {
		fb_array->values[i].value = bpf_max(
			fb_array->values[i].min,
			fb_array->values[i].max,
			fft
		) / max_amplitude;
	}
}

void fft_to_bands_linear(t_fft* fft, t_frequency_band_array* fb_array)
{
	double max_amplitude = (1 << FFT_MAGNITUDE_MAX_DIGITS) - 1;

	init_bands_linear(fb_array, 20, 20000);

	for (int i = 0; i < fb_array->length; i++) {
		fb_array->values[i].value = bpf_max(
			fb_array->values[i].min,
			fb_array->values[i].max,
			fft
		) / max_amplitude;
	}
}
