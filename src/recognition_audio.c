#include "recognition_audio.h"
#include "log/src/log.h"

int is_kick(t_fft* fft) {
   double max_amplitude = pow(2, 18) - 1;
	double val = bpf_average(80, 120, fft) / max_amplitude;

   double value_cropped;
   if (val> 1) {
		value_cropped = 1;
	} else if (val< 0) {
		value_cropped = 0;
	} else {
		value_cropped = val;
	}

   /* char str[100];
   sprintf(str, "%f", value_cropped);
   log_info(str); */

   if (value_cropped > 0.4) {
      return 1;
   }
   return 0;
}
