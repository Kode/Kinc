#pragma once

#include <kinc/global.h>

#include <kinc/audio2/audio.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_a2_buffer_format_t format;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float my_volume;
	bool in_use;
} kinc_a1_sound_t;

KINC_FUNC kinc_a1_sound_t *kinc_a1_sound_create(const char *filename);
KINC_FUNC void kinc_a1_sound_destroy(kinc_a1_sound_t *sound);
KINC_FUNC float kinc_a1_sound_volume(kinc_a1_sound_t *sound);
KINC_FUNC void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value);

#ifdef __cplusplus
}
#endif
