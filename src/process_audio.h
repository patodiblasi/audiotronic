#ifndef PROCESS_AUDIO_INCLUDED
#define PROCESS_AUDIO_INCLUDED

typedef struct {
	unsigned int min_freq;
	unsigned int max_freq;
	unsigned int min_sample_rate;
	unsigned int min_samples;
	float min_samples_duration_ms;
} audio_config;

audio_config new_audio_config(int min_freq, int max_freq);

#endif