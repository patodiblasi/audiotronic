#include "requests.h"

char base_url[] = "http://192.168.1.14";

CURL* curl;

void init_requests() {
   curl = curl_easy_init();
   curl_easy_setopt(curl, CURLOPT_POST, 1L);
   curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
}

void turn_on() {
   curl_easy_setopt(curl, CURLOPT_URL, strcat(base_url, "/on"));
   curl_easy_perform(curl);
}

void turn_off() {
   curl_easy_setopt(curl, CURLOPT_URL, strcat(base_url, "/off"));
   curl_easy_perform(curl);
}

void cleanup_requests() {
   curl_easy_cleanup(curl);
}
