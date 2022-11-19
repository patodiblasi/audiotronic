#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "helpers.h"

void formatFreq(char* dest, double freq)
{
	if (freq >= 1000) {
		sprintf(dest, "%ld", lround(freq / 1000));
		strncat(dest, "K", 1);
	}

	sprintf(dest, "%d", (int)freq);
}

FILE* redirect_stderr()
{
	// [0] lectura, [1] escritura
	int p[2];

	if (pipe(p) != 0) {
		fprintf(stderr, "pipe: No fue posible redirigir stderr.\n");
		return NULL;
	}

	if (fcntl(p[0], F_SETFL, O_NONBLOCK) < 0) {
		fprintf(stderr, "fcntl: No fue posible redirigir stderr.\n");
		return NULL;
	}

	if (dup2(p[1], STDERR_FILENO) == -1) {
		fprintf(stderr, "dup2: No fue posible redirigir stderr.\n");
		return NULL;
	}

	FILE* redirected = fdopen(p[0], "r");
	if (!redirected) {
		// Vuelvo a dejar todo como estaba:
		dup2(STDERR_FILENO, p[1]);
		fprintf(stderr, "fdopen: No fue posible redirigir stderr.\n");
		return NULL;
	}

	return redirected;
}
