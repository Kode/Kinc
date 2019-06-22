#pragma once

#include <stdbool.h>
#include <stdint.h>

struct stb_vorbis;

typedef struct {
	struct stb_vorbis *vorbis;
	int chans;
	int rate;
	bool myLooping;
	float myVolume;
	bool decoded;
	bool rateDecodedHack;
	bool end;
	float samples[2];
	uint8_t *buffer;
} kinc_a1_sound_stream_t;

kinc_a1_sound_stream_t *kinc_a1_sound_stream_create(const char *filename, bool looping);
float kinc_a1_sound_stream_next_sample(kinc_a1_sound_stream_t *stream);
int kinc_a1_sound_stream_channels(kinc_a1_sound_stream_t *stream);
int kinc_a1_sound_stream_sample_rate(kinc_a1_sound_stream_t *stream);
bool kinc_a1_sound_stream_looping(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_set_looping(kinc_a1_sound_stream_t *stream, bool loop);
bool kinc_a1_sound_stream_ended(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_length(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_position(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_reset(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_volume(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_set_volume(kinc_a1_sound_stream_t *stream, float value);
