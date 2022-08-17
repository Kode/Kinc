#include <kinc/audio2/audio.h>

static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = NULL;
static kinc_a2_buffer_t a2_buffer;

void kinc_a2_init() {}

void kinc_a2_update() {}

void kinc_a2_shutdown() {}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
