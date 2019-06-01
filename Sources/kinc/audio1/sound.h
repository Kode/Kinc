#pragma once

#include <kinc/audio2/audio.h>

#include <stdint.h>

typedef struct {
	kinc_a2_buffer_format_t format;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float my_volume;
} kinc_a1_sound_t;

kinc_a1_sound_t *kinc_a1_sound_create(const char *filename);
void kinc_a1_sound_destroy(kinc_a1_sound_t *sound);
float kinc_a1_sound_volume(kinc_a1_sound_t *sound);
void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value);
