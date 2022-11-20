#ifndef IS_REQUESTS_INCLUDED
#define IS_REQUESTS_INCLUDED

#include <curl/curl.h>
#include <string.h>

int init_requests();
void turn_on();
void turn_off();
void get_server_variables();
void perform_requests();

#endif
