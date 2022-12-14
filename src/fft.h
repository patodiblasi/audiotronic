#ifndef IS_FFT_INCLUDED
#define IS_FFT_INCLUDED

#include <math.h>

// The arrays for the fft will be computed in place
// and thus your array will have the fft result
// written over your original data.
// We require an array of real and imaginary doubles
// where they are both of length N
void fft_compute(double data_re[], double data_im[], const unsigned int N);

// helper functions called by the fft
// data will first be rearranged then computed
// an array of  {1, 2, 3, 4, 5, 6, 7, 8} will be
// rearranged to {1, 5, 3, 7, 2, 6, 4, 8}
void rearrange(double data_re[], double data_im[], const unsigned int N);

// the heavy lifting of computation
void compute(double data_re[], double data_im[], const unsigned int N);

void fft_amplitude_to_magnitude(double *fft_real, double *fft_imaginary, int fft_length);

#endif
