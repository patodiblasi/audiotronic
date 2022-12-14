
#include "fft.h"

void fft_compute(double data_re[], double data_im[], const unsigned int N)
{
	rearrange(data_re, data_im, N);
	compute(data_re, data_im, N);
}

void rearrange(double data_re[], double data_im[], const unsigned int N)
{
	unsigned int target = 0;
	for(unsigned int position=0; position<N;position++)
	{
		if(target>position) {
			// Swap target y position
			const double temp_re = data_re[target];
			const double temp_im = data_im[target];
			data_re[target] = data_re[position];
			data_im[target] = data_im[position];
			data_re[position] = temp_re;
			data_im[position] = temp_im;
		}
		unsigned int mask = N;
		while(target & (mask >>=1))
			target &= ~mask;
		target |= mask;
	}
}

void compute(double data_re[], double data_im[], const unsigned int N)
{
	const double pi = -3.14159265358979323846;

	for(unsigned int step=1; step<N; step <<=1) {
		const unsigned int jump = step << 1;
		const double step_d = (double) step;
		double twiddle_re = 1.0;
		double twiddle_im = 0.0;
		for(unsigned int group=0; group<step; group++)
		{
			for(unsigned int pair=group; pair<N; pair+=jump)
			{
				const unsigned int match = pair + step;
				if (match >= N) {
					continue;
				}
				const double product_re = twiddle_re*data_re[match]-twiddle_im*data_im[match];
				const double product_im = twiddle_im*data_re[match]+twiddle_re*data_im[match];
				data_re[match] = data_re[pair]-product_re;
				data_im[match] = data_im[pair]-product_im;
				data_re[pair] += product_re;
				data_im[pair] += product_im;
			}

			// we need the factors below for the next iteration
			// if we don't iterate then don't compute
			if(group+1 == step)
			{
				continue;
			}

			double angle = pi*((double) group+1)/step_d;
			twiddle_re = cos(angle);
			twiddle_im = sin(angle);
		}
	}
}

void fft_amplitude_to_magnitude(double *fft_real, double *fft_imaginary, int fft_length)
{
	for (int i=0; i < fft_length; i++) {
		// Básicamente, estaría calculando la hipotenusa
		fft_real[i] = sqrt(fft_real[i] * fft_real[i] + fft_imaginary[i] * fft_imaginary[i]);
	}
}

