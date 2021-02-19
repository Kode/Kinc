#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

KINC_FUNC kinc_a1_sound_stream_t *kinc_a1_sound_stream_create(const char *filename, bool looping);
KINC_FUNC float kinc_a1_sound_stream_next_sample(kinc_a1_sound_stream_t *stream);
KINC_FUNC int kinc_a1_sound_stream_channels(kinc_a1_sound_stream_t *stream);
KINC_FUNC int kinc_a1_sound_stream_sample_rate(kinc_a1_sound_stream_t *stream);
KINC_FUNC bool kinc_a1_sound_stream_looping(kinc_a1_sound_stream_t *stream);
KINC_FUNC void kinc_a1_sound_stream_set_looping(kinc_a1_sound_stream_t *stream, bool loop);
KINC_FUNC bool kinc_a1_sound_stream_ended(kinc_a1_sound_stream_t *stream);
KINC_FUNC float kinc_a1_sound_stream_length(kinc_a1_sound_stream_t *stream);
KINC_FUNC float kinc_a1_sound_stream_position(kinc_a1_sound_stream_t *stream);
KINC_FUNC void kinc_a1_sound_stream_reset(kinc_a1_sound_stream_t *stream);
KINC_FUNC float kinc_a1_sound_stream_volume(kinc_a1_sound_stream_t *stream);
KINC_FUNC void kinc_a1_sound_stream_set_volume(kinc_a1_sound_stream_t *stream, float value);

#ifdef __cplusplus
}
#endif
