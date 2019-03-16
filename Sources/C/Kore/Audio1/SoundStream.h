#pragma once

#include <stdbool.h>
#include <stdint.h>

struct stb_vorbis;

typedef struct {
	struct stb_vorbis* vorbis;
	int chans;
	int rate;
	bool myLooping;
	float myVolume;
	bool decoded;
	bool rateDecodedHack;
	bool end;
	float samples[2];
	uint8_t *buffer;
} Kore_A1_SoundStream;

Kore_A1_SoundStream *Kore_A1_CreateSoundStream(const char *filename, bool looping);
float Kore_A1_SoundStreamNextSample(Kore_A1_SoundStream *stream);
int Kore_A1_SoundStreamChannels(Kore_A1_SoundStream *stream);
int Kore_A1_SoundStreamSampleRate(Kore_A1_SoundStream *stream);
bool Kore_A1_SoundStreamLooping(Kore_A1_SoundStream *stream);
void Kore_A1_SoundStreamSetLooping(Kore_A1_SoundStream *stream, bool loop);
bool Kore_A1_SoundStreamEnded(Kore_A1_SoundStream *stream);
float Kore_A1_SoundStreamLength(Kore_A1_SoundStream *stream);
float Kore_A1_SoundStreamPosition(Kore_A1_SoundStream *stream);
void Kore_A1_SoundStreamReset(Kore_A1_SoundStream *stream);
float Kore_A1_SoundStreamVolume(Kore_A1_SoundStream *stream);
void Kore_A1_SoundStreamSetVolume(Kore_A1_SoundStream *stream, float value);
