#include "audio.h"

#include <stdbool.h>

typedef struct kinc_a1_sound {
	kinc_a2_buffer_format_t format;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float volume;
	bool in_use;
} kinc_a1_sound_t;

#include "audio.c.h"
#include "sound.c.h"
#include "soundstream.c.h"
