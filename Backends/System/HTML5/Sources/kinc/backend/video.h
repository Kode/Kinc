#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int nothing;
} kinc_video_impl_t;

typedef struct kinc_internal_video_sound_stream {
	int nothing;
} kinc_internal_video_sound_stream_t;

void kinc_video_sound_stream_impl_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency);

void kinc_video_sound_stream_impl_destroy(kinc_internal_video_sound_stream_t *stream);

void kinc_video_sound_stream_impl_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count);

float kinc_video_sound_stream_impl_next_sample(kinc_internal_video_sound_stream_t *stream);

bool kinc_video_sound_stream_impl_ended(kinc_internal_video_sound_stream_t *stream);

#ifdef __cplusplus
}
#endif
