#include "envs.h"

char *envs[] = { "AUDIOTRONIC_DRIVER", "AUDIOTRONIC_INPUT", "AUDIOTRONIC_SERVER" };

int check_envs() {
	int envs_length = sizeof(envs) / sizeof(envs[0]);

	for (int i = 0; i < envs_length; i++) {
		char* value = getenv(envs[i]);

		// Make sure envar actually exists
		if (!value) {
			fprintf(stderr, "\nNo se encontró la variable de entorno %s.", envs[i]);
			return 0;
		}
	}
}
