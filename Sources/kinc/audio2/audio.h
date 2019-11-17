#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int channels;
	int samples_per_second;
	int bits_per_sample;
} kinc_a2_buffer_format_t;

typedef struct {
	kinc_a2_buffer_format_t format;
	uint8_t *data;
	int data_size;
	int read_location;
	int write_location;
} kinc_a2_buffer_t;

void kinc_a2_init();
void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples));
void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)());
extern int kinc_a2_samples_per_second;
void kinc_a2_update();
void kinc_a2_shutdown();

#ifdef __cplusplus
}
#endif
