#include <string.h>
#include <stdio.h>
#include <math.h>
#include "helpers.h"

void formatFreq(char* dest, double freq)
{
	if (freq >= 1000) {
		sprintf(dest, "%ld", lround(freq / 1000));
		strncat(dest, "K", 1);
	}

	sprintf(dest, "%d", (int)freq);
}
