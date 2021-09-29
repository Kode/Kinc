#pragma once

#include <kinc/global.h>

#include <kinc/audio2/audio.h>

#include <stdint.h>

/*! \file sound.h
    \brief Sounds are pre-decoded on load and therefore primarily useful for playing sound-effects.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_a1_sound;
typedef struct kinc_a1_sound kinc_a1_sound_t;

/// <summary>
/// Create a sound from a wav file.
/// </summary>
/// <param name="filename">Path to a wav file</param>
/// <returns>The newly created sound</returns>
KINC_FUNC kinc_a1_sound_t *kinc_a1_sound_create(const char *filename);

/// <summary>
/// Destroy a sound.
/// </summary>
/// <param name="sound">The sound to destroy.</param>
KINC_FUNC void kinc_a1_sound_destroy(kinc_a1_sound_t *sound);

/// <summary>
/// Gets the current volume-multiplicator that's used when playing the sound.
/// </summary>
/// <param name="sound">The sound to query</param>
/// <returns>The volume-multiplicator</returns>
KINC_FUNC float kinc_a1_sound_volume(kinc_a1_sound_t *sound);

/// <summary>
/// Sets the volume-multiplicator that's used when playing the sound.
/// </summary>
/// <param name="sound">The sound to modify</param>
/// <param name="value">The volume-multiplicator to set</param>
KINC_FUNC void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value);

#ifdef __cplusplus
}
#endif
