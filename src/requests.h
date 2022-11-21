#ifndef IS_REQUESTS_INCLUDED
#define IS_REQUESTS_INCLUDED

#include <curl/curl.h>
#include <string.h>
#include "config.h"

struct memory {
   size_t size;
   char* response;
};

int init_requests();
void turn_on();
void turn_off();
void get_server_variables(t_drop_params* config);
void perform_requests();
void parse_pseudo_json_response(char* response, t_drop_params* config);
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp);

#endif
