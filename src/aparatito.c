#include "aparatito.h"

int is_on = 0;

int run_aparatito_frame(t_fft* fft) {
   if (is_kick(fft)) {
      if (is_on == 0) {
         turn_on();
         is_on = 1;
      }
   } else {
      if (is_on == 1) {
         turn_off();
         is_on = 0;
      }
   }
   return 1;
}



void get_aparatito_variables(t_drop_params* config) {
   // hacer GET al servidor, responde con valores
   // mapear valores a struct
}
