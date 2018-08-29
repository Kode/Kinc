#pragma once

#include "Sound.h"
#include "SoundStream.h"

#include <stdbool.h>

struct _Kore_VideoSoundStream;

typedef struct {
	Sound* sound;
	float position;
	bool loop;
	float volume;
	float pitch;
} Kore_A1_Channel;

typedef struct {
	SoundStream* stream;
	int position;
} Kore_A1_StreamChannel;

typedef struct {
	VideoSoundStream* stream;
	int position;
} Kore_A1_VideoChannel;

void Kore_A1_Init();
Kore_A1_Channel* Kore_A1_Play(Kore_A1_Sound* sound, bool loop = false, float pitch = 1.0f, bool unique = false);
void Kore_A1_Stop(Kore_A1_Sound* sound);
void Kore_A1_Play(Kore_A1_SoundStream *stream);
void Kore_A1_Stop(Kore_A1_SoundStream *stream);
void Kore_A1_Play(Kore_A1_VideoSoundStream *stream);
void Kore_A1_Stop(Kore_A1_VideoSoundStream *stream);
void Kore_Internal_A1_Mix(int samples);
