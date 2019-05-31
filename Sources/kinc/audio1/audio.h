#pragma once

#include "sound.h"
#include "soundstream.h"

#include <stdbool.h>

struct Kinc_A1_VideoSoundStream;

typedef struct {
	Kinc_A1_Sound *sound;
	float position;
	bool loop;
	float volume;
	float pitch;
} Kinc_A1_Channel;

typedef struct {
	Kinc_A1_SoundStream *stream;
	int position;
} Kinc_A1_StreamChannel;

typedef struct {
	struct Kinc_A1_VideoSoundStream *stream;
	int position;
} Kinc_A1_VideoChannel;

void Kinc_A1_Init();
Kinc_A1_Channel *Kinc_A1_PlaySound(Kinc_A1_Sound *sound, bool loop, float pitch,
                                   bool unique); //(Kinc_A1_Sound* sound, bool loop = false, float pitch = 1.0f, bool unique = false);
void Kinc_A1_StopSound(Kinc_A1_Sound *sound);
void Kinc_A1_PlaySoundStream(Kinc_A1_SoundStream *stream);
void Kinc_A1_StopSoundStream(Kinc_A1_SoundStream *stream);
void Kinc_A1_PlayVideoSoundStream(struct Kinc_A1_VideoSoundStream *stream);
void Kinc_A1_StopVideoSoundStream(struct Kinc_A1_VideoSoundStream *stream);
void Kinc_Internal_A1_Mix(Kinc_A2_Buffer *buffer, int samples);
