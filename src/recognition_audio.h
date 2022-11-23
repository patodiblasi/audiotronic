#include "process_audio.h"
#include "config.h"

int is_kick(t_fft* fft, t_drop_params* config);
int is_drop(t_fft* fft, t_drop_params* config);
static double compute_band_value(t_fft* fft, t_drop_params* config);
