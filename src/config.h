#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
	char audio_driver[100];
	char audio_device[100];
	char server[100];
} t_audiotronic_config;

int read_env_config(t_audiotronic_config* config);

#endif