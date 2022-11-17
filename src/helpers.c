#include <string.h>
#include "helpers.h"
#include "show_audio.h"

void formatFreq(char* dest, double freq) {
    if (freq >= 1000) {
        sprintf(dest, "%ld", lround(freq / 1000));
        strncat(dest, "K", 1);
    }
    
    sprintf(dest, "%d", (int)freq);
}
