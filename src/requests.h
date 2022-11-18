#ifndef IS_REQUESTS_INCLUDED
#define IS_REQUESTS_INCLUDED

#include <curl/curl.h>
#include <string.h>

void init_requests();
void turn_on();
void turn_off();
void cleanup_requests();

#endif
