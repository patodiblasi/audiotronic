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

void parse_pseudo_json_response(char* response, t_drop_params* config) {
   char delim[10] = ",:";
	char *token = strtok(response, delim);
   int pos = 0;
   char param[100];

	while (token != NULL)
	{     
      if (pos % 2 == 0) {  // nombre del parametro
         strcpy(param, token);
      }
      if (pos % 2 != 0) { // valor
         int val = atoi(token);
         if (strcmp(param, "min_freq") == 0)
            config->min_freq = val;
         if (strcmp(param, "max_freq") == 0)
            config->max_freq = val;
         if (strcmp(param, "inactivity_time") == 0)
            config->inactivity_time = val;
         if (strcmp(param, "output_time") == 0)
            config->output_time = val;
         if (strcmp(param, "threshold") == 0)
            config->threshold = val;
      }
		// printf("%d\n", config->min_freq);
		token = strtok(NULL, delim);
      pos++;
	}
}

static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
   size_t realsize = size * nmemb;
   struct memory *mem = (struct memory *)userp;

   void* ptr = realloc(mem->response, mem->size + realsize + 1);
   if(ptr == NULL)
      return 0;  /* out of memory! */

   mem->response = (char*)ptr;
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;

   return realsize;
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
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

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
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

   curl_multi_add_handle(multi_handle, curl);
 	
	perform_requests();

	curl_multi_remove_handle(multi_handle, curl);
	curl_easy_cleanup(curl);
}

void get_server_variables(t_drop_params* config) {
	if (!is_server_configured) return;

	char url[100];
	strcpy(url, base_url);
	strcat(url, "/variables");

   struct memory chunk = {0};

	CURL* curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

   curl_multi_add_handle(multi_handle, curl);
	
	perform_requests();
	parse_pseudo_json_response(chunk.response, config);
	
	curl_multi_remove_handle(multi_handle, curl);
	curl_easy_cleanup(curl);
}
