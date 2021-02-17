#pragma once

#include <kinc/graphics4/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int nothing;
} kinc_video_impl_t;

void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

void kinc_video_play(kinc_video_t *video) {}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

int kinc_video_width(kinc_video_t *video) { return 256; }

int kinc_video_height(kinc_video_t *video) { return 256; }

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) { return NULL; }

double kinc_video_duration(kinc_video_t *video) { return 0.0; }

double kinc_video_position(kinc_video_t *video) { return 0.0; }

bool kinc_video_finished(kinc_video_t *video) { return false; }

bool kinc_video_paused(kinc_video_t *video) { return false; }

void kinc_video_update(kinc_video_t *video, double time) {}

#ifdef __cplusplus
}
#endif
