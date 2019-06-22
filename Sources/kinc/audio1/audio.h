#pragma once

#include "sound.h"
#include "soundstream.h"

#include <stdbool.h>

struct kinc_a1_video_sound_stream;

typedef struct {
	kinc_a1_sound_t *sound;
	float position;
	bool loop;
	float volume;
	float pitch;
} kinc_a1_channel_t;

typedef struct {
	kinc_a1_sound_stream_t *stream;
	int position;
} kinc_a1_stream_channel_t;

typedef struct {
	struct kinc_a1_video_sound_stream *stream;
	int position;
} kinc_a1_video_channel_t;

void kinc_a1_init();
kinc_a1_channel_t *kinc_a1_play_sound(kinc_a1_sound_t *sound, bool loop, float pitch,
                                   bool unique); //(kinc_a1_sound_t *sound, bool loop = false, float pitch = 1.0f, bool unique = false);
void kinc_a1_stop_sound(kinc_a1_sound_t *sound);
void kinc_a1_play_sound_stream(kinc_a1_sound_stream_t *stream);
void kinc_a1_stop_sound_stream(kinc_a1_sound_stream_t *stream);
void kinc_a1_play_video_sound_stream(struct kinc_a1_video_sound_stream *stream);
void kinc_a1_stop_video_sound_stream(struct kinc_a1_video_sound_stream *stream);
void kinc_internal_a1_mix(kinc_a2_buffer_t *buffer, int samples);
