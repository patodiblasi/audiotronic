#include <stdio.h>
#include <math.h>
#include <complex.h>

#ifndef PI
	#define PI 3.14159265358979323846
#endif

typedef double complex cplx;

void _fft(cplx buf[], cplx out[], int n, int step);
void fft(cplx buf[], int n);
void fft_show(const char * s, cplx buf[]);
