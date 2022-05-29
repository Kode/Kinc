#pragma once

#include <kinc/global.h>

#include "sound.h"
#include "soundstream.h"

#include <stdbool.h>

/*! \file audio.h
    \brief Audio1 is a high-level audio-API that lets you directly play audio-files. Depending on the target-system it either sits directly on a high-level
   system audio-API or is implemented based on Audio2.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_internal_video_sound_stream;

struct kinc_a1_channel;
typedef struct kinc_a1_channel kinc_a1_channel_t;

struct kinc_a1_stream_channel;
typedef struct kinc_a1_stream_channel kinc_a1_stream_channel_t;

struct kinc_internal_video_channel;
typedef struct kinc_internal_video_channel kinc_internal_video_channel_t;

/// <summary>
/// Initialize the Audio1-API.
/// </summary>
KINC_FUNC void kinc_a1_init(void);

/// <summary>
/// Plays a sound immediately.
/// </summary>
/// <param name="sound">The sound to play</param>
/// <param name="loop">Whether or not to automatically loop the sound</param>
/// <param name="pitch">Changes the pitch by providing a value that's not 1.0f</param>
/// <param name="unique">Makes sure that a sound is not played more than once at the same time</param>
/// <returns>A channel object that can be used to control the playing sound</returns>
KINC_FUNC kinc_a1_channel_t *kinc_a1_play_sound(kinc_a1_sound_t *sound, bool loop, float pitch, bool unique);

/// <summary>
/// Stops the sound from playing.
/// </summary>
/// <param name="sound">The sound to stop</param>
KINC_FUNC void kinc_a1_stop_sound(kinc_a1_sound_t *sound);

/// <summary>
/// Starts playing a sound-stream.
/// </summary>
/// <param name="stream">The stream to play</param>
KINC_FUNC void kinc_a1_play_sound_stream(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Stops a sound-stream from playing.
/// </summary>
/// <param name="stream">The stream to stop.</param>
KINC_FUNC void kinc_a1_stop_sound_stream(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Gets the current volume of a channel. Only works correctly if the channel is still playing.
/// </summary>
/// <param name="channel">The channel to get the volume from</param>
/// <returns>The volume</returns>
KINC_FUNC float kinc_a1_channel_get_volume(kinc_a1_channel_t *channel);

/// <summary>
/// Sets the current volume of a channel. Only works correctly if the channel is still playing.
/// </summary>
/// <param name="channel">The channel to set the volume for</param>
/// <param name="volume">The volume to set</param>
/// <returns></returns>
KINC_FUNC void kinc_a1_channel_set_volume(kinc_a1_channel_t *channel, float volume);

void kinc_internal_play_video_sound_stream(struct kinc_internal_video_sound_stream *stream);
void kinc_internal_stop_video_sound_stream(struct kinc_internal_video_sound_stream *stream);
void kinc_internal_a1_mix(kinc_a2_buffer_t *buffer, int samples);

#ifdef __cplusplus
}
#endif
