#include "requests.h"
#include <stdlib.h>

char base_url[100];
int is_server_configured = 0;

CURLM* multi_handle;

int init_requests() {
	char* base_url_env = getenv("AUDIOTRONIC_SERVER");
	if (base_url_env) {
  	multi_handle = curl_multi_init();

		// Copio porque el resultado de getenv puede ser sobrescrito, según la doc
		strcpy(base_url, base_url_env);
		is_server_configured = 1;
	} else {
		is_server_configured = 0;
	}

	return is_server_configured;
}

void perform_requests() {
	int still_running = 0;
	do {
  	CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
		//if(still_running)
  		/* wait for activity, timeout or "nothing" */
  		//mc = curl_multi_poll(multi_handle, NULL, 0, 1000, NULL);
		if(mc)
  		break;
  } while(still_running);
}

void turn_on() {
	if (!is_server_configured) return;

	char url[100];
	strcpy(url, base_url);
	strcat(url, "/on");

	CURL* curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

  curl_multi_add_handle(multi_handle, curl);

	perform_requests();

	curl_multi_remove_handle(multi_handle, curl);
	curl_easy_cleanup(curl);
}

void turn_off() {
	if (!is_server_configured) return;

	char url[100];
	strcpy(url, base_url);
	strcat(url, "/off");

	CURL* curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_multi_add_handle(multi_handle, curl);
 	
	perform_requests();

	curl_multi_remove_handle(multi_handle, curl);
	curl_easy_cleanup(curl);
}

void get_server_variables() {
	if (!is_server_configured) return;

	char url[100];
	strcpy(url, base_url);
	strcat(url, "/variables");

	CURL* curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

  curl_multi_add_handle(multi_handle, curl);
	
	perform_requests();

	curl_multi_remove_handle(multi_handle, curl);
	curl_easy_cleanup(curl);
}
