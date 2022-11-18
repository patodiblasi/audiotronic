#include "requests.h"
#include <stdlib.h>

char base_url[100];

CURL* curl;
int is_server_configured = 0;

int init_requests() {
	char* base_url_env = getenv("AUDIOTRONIC_SERVER");

	if (base_url_env) {
		// Copio porque el resultado de getenv puede ser sobrescrito, según la doc
		strcpy(base_url, base_url_env);

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

		is_server_configured = 1;
	} else {
		is_server_configured = 0;
	}

	return is_server_configured;
}

void turn_on() {
	if (!is_server_configured) return;

	curl_easy_setopt(curl, CURLOPT_URL, strcat(base_url, "/on"));
	curl_easy_perform(curl);
}

void turn_off() {
	if (!is_server_configured) return;

	curl_easy_setopt(curl, CURLOPT_URL, strcat(base_url, "/off"));
	curl_easy_perform(curl);
}

void cleanup_requests() {
	if (!is_server_configured) return;

	curl_easy_cleanup(curl);
}
