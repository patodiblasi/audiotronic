#include "aparatito.h"

int is_on = 0;

int run_aparatito_frame(t_fft* fft, t_drop_params* config) {
   if (is_kick(fft, config)) {
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
   get_server_variables(config);
}
