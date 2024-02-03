#include <kinc/audio2/audio.h>
#include <stdlib.h>

static void (*a2_callback)(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata) = NULL;
static void *a2_userdata = NULL;
static kinc_a2_buffer_t a2_buffer;

void kinc_a2_init() {}

void kinc_a2_update() {}

void kinc_a2_shutdown() {}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata), void *userdata) {
	a2_callback = kinc_a2_audio_callback;
	a2_userdata = userdata;
}

static uint32_t samples_per_second = 44100;
static void (*sample_rate_callback)(void *userdata) = NULL;
static void *sample_rate_callback_userdata = NULL;

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}

void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void *userdata), void *userdata) {
	sample_rate_callback_userdata = userdata;
	sample_rate_callback = kinc_a2_sample_rate_callback;
}
