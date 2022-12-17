#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
	char audio_driver[100];
	char audio_device[100];
	char server[100];
} t_audiotronic_config;

typedef struct {
   int threshold; // de 0 a 1023 10 bits
   int min_freq; // en hz
   int max_freq; // en hz
   int inactivity_time; // en segundos
   int output_time; // en segundos
} t_drop_params;

int read_env_config(t_audiotronic_config* config);

#endif
