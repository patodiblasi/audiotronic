#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "log/src/log.h"

int read_env_config(t_audiotronic_config* config)
{
	char* tmp;

	// Copio porque el resultado de getenv puede ser sobrescrito, según la doc,
	// ya que el string no está en memoria que yo haya reservado.
	tmp = getenv("AUDIOTRONIC_DRIVER");
	if (!tmp) {
		log_error("Falta la variable de entorno AUDIOTRONIC_DRIVER.");
		return 0;
	}
	strcpy(config->audio_driver, tmp);

	tmp = getenv("AUDIOTRONIC_INPUT");
	if (!tmp) {
		log_error("Falta la variable de entorno AUDIOTRONIC_INPUT.");
		return 0;
	}
	strcpy(config->audio_device, tmp);

	tmp = getenv("AUDIOTRONIC_SERVER");
	if (!tmp) {
		log_warn("Falta la variable de entorno AUDIOTRONIC_SERVER.");
		return 1;
	}
	strcpy(config->server, tmp);

	return 1;
}
