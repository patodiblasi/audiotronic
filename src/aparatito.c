#include "aparatito.h"

int is_on = 0;

int run_aparatito_frame(t_fft* fft, t_drop_params* config) {
   if (is_drop(fft, config)) {
      if (!is_on) {
         turn_on();
         is_on = 1;
      }
   } else {
      if (is_on) {
         turn_off();
         is_on = 0;
      }
   }
   return 1;
}



void get_aparatito_variables(t_drop_params* config) {
   log_info("Leyendo variables del servidor");
   get_server_variables(config);
}
