#include "recognition_audio.h"

double compute_band_value(t_fft* fft, t_drop_params* config)
{
   double max_amplitude =  (1 << 18) - 1;
	double val = bpf_average(config->min_freq, config->max_freq, fft) / max_amplitude;
   double value_cropped;

   if (val > 1) {
		value_cropped = 1.0;
	} else if (val< 0) {
		value_cropped = 0.0;
	} else {
		value_cropped = val;
	}
   return value_cropped;
}

int is_kick(t_fft* fft, t_drop_params* config)
{
   double band_value = compute_band_value(fft, config);
   double threshold = config->threshold / 1024.0;

   if (band_value >= threshold) {
      return 1;
   }
   return 0;
}

int frames_without_kick = 0;  // En frames o similar
int elapsed_frames_since_drop = 0;
int was_on = 0;

int is_drop(t_fft* fft, t_drop_params* config) 
{
   const int frames_to_time = 50;  // Valor aproximado para transformar frames en segundos

   if (was_on && config->output_time >= elapsed_frames_since_drop / frames_to_time) {  // Si ya se encendio y todavia no paso el tiempo para que se apague
      elapsed_frames_since_drop++;  // Incremento frame en este scope
      return 1;  // Envio señal al server para prender
   } else {  // Sino reseteo flags
      elapsed_frames_since_drop = 0;
      was_on = 0;
   }
   if (is_kick(fft, config)) {  // Si hay kick
      if(frames_without_kick / frames_to_time >= config->inactivity_time) {  // Y el tiempo sin kick que paso es mayor a x 
         frames_without_kick = 0;  // Reinicio el conteo de tiempo sin kick
         was_on = 1;
         log_info("DROP!");
         return 1;  // Debo enviar señal al servidr.
      }
      frames_without_kick = 0;  // Reinicio el conteo de tiempo sin kick
      return 0;
   }
   // Si no hubo kick
   char msg[100]; // Logeo msj para saber donde estamos parados
   int countdown = config->inactivity_time - frames_without_kick / frames_to_time;
   if(countdown == config->inactivity_time) {
      sprintf(msg, "Kick mode...Esperando bajada");
   } else if (countdown > 0) {
      sprintf(msg, "Entrando en bajada mode en %is", countdown);
   } else {
      sprintf(msg, "Bajada mode...Esperando drop entre %i-%i Hz, mayor a %.f porciento", config->min_freq, config->max_freq, config->threshold / 1024.0f * 100);
   }
   log_info(msg);
   
   frames_without_kick++;  // Incrementar tiempo sin kick
   return 0;  // Seguro que la salida deberia estar apagada
}
