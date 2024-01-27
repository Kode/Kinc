#pragma once

#include <kinc/global.h>

#include <stdint.h>

/*! \file audio.h
    \brief Audio2 is a low-level audio-API that allows you to directly provide a stream of audio-samples.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_a2_buffer_format {
	int channels;
	int samples_per_second;
	int bits_per_sample;
} kinc_a2_buffer_format_t;

typedef struct kinc_a2_buffer {
	kinc_a2_buffer_format_t format;
	uint8_t *data;
	int data_size;
	int read_location;
	int write_location;
} kinc_a2_buffer_t;

/// <summary>
/// Initializes the Audio2-API.
/// </summary>
KINC_FUNC void kinc_a2_init(void);

/// <summary>
/// Sets the callback that's used to provide audio-samples. This is the primary method of operation for Audio2. The callback is expected to write the requested
/// number of samples into the ring-buffer. The callback is typically called from the system's audio-thread to minimize audio-latency.
/// </summary>
/// <param name="kinc_a2_audio_callback">The callback to set</param>
/// <param name="userdata">The user data provided to the callback</param>
KINC_FUNC void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples, void *userdata), void *userdata);

/// <summary>
/// Sets a callback that's called when the system's sample-rate changes.
/// </summary>
/// <param name="kinc_a2_sample_rate_callback">The callback to set</param>
/// <param name="userdata">The user data provided to the callback</param>
/// <returns></returns>
KINC_FUNC void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void *userdata), void *userdata);

/// <summary>
/// The current sample-rate of the system.
/// </summary>
KINC_FUNC extern int kinc_a2_samples_per_second;

/// <summary>
/// kinc_a2_update should be called every frame. It is required by some systems to pump their audio-loops but on most systems it is a no-op.
/// </summary>
KINC_FUNC void kinc_a2_update(void);

/// <summary>
/// Shuts down the Audio2-API.
/// </summary>
KINC_FUNC void kinc_a2_shutdown(void);

#ifdef KINC_IMPLEMENTATION_AUDIO2
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

int kinc_a2_samples_per_second = 44100;

// BACKENDS-PLACEHOLDER

#endif

#ifdef __cplusplus
}
#endif
