#include "recognition_audio.h"
#include "log/src/log.h"

int is_kick(t_fft* fft, t_drop_params* config) {
   double max_amplitude = pow(2, 18) - 1;
	double val = bpf_average(config->min_freq, config->max_freq, fft) / max_amplitude;
   double value_cropped;
   if (val> 1) {
		value_cropped = 1;
	} else if (val< 0) {
		value_cropped = 0;
	} else {
		value_cropped = val;
	}

   float threshold = config->threshold / 1024.0f;
   if (value_cropped > threshold) {
      return 1;
   }
   return 0;
}
