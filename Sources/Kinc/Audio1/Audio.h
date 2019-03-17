#pragma once

#include "Sound.h"
#include "SoundStream.h"

#include <stdbool.h>

struct Kore_A1_VideoSoundStream;

typedef struct {
	Kore_A1_Sound *sound;
	float position;
	bool loop;
	float volume;
	float pitch;
} Kore_A1_Channel;

typedef struct {
	Kore_A1_SoundStream *stream;
	int position;
} Kore_A1_StreamChannel;

typedef struct {
	struct Kore_A1_VideoSoundStream *stream;
	int position;
} Kore_A1_VideoChannel;

void Kore_A1_Init();
Kore_A1_Channel *Kore_A1_PlaySound(Kore_A1_Sound *sound, bool loop, float pitch, bool unique); //(Kore_A1_Sound* sound, bool loop = false, float pitch = 1.0f, bool unique = false);
void Kore_A1_StopSound(Kore_A1_Sound* sound);
void Kore_A1_PlaySoundStream(Kore_A1_SoundStream *stream);
void Kore_A1_StopSoundStream(Kore_A1_SoundStream *stream);
void Kore_A1_PlayVideoSoundStream(struct Kore_A1_VideoSoundStream *stream);
void Kore_A1_StopVideoSoundStream(struct Kore_A1_VideoSoundStream *stream);
void Kore_Internal_A1_Mix(Kore_A2_Buffer *buffer, int samples);
