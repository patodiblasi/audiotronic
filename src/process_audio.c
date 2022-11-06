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
	#if FFT_MODULE_ARDUINO
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

