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
} Kinc_A1_SoundStream;

Kinc_A1_SoundStream *Kinc_A1_CreateSoundStream(const char *filename, bool looping);
float Kinc_A1_SoundStreamNextSample(Kinc_A1_SoundStream *stream);
int Kinc_A1_SoundStreamChannels(Kinc_A1_SoundStream *stream);
int Kinc_A1_SoundStreamSampleRate(Kinc_A1_SoundStream *stream);
bool Kinc_A1_SoundStreamLooping(Kinc_A1_SoundStream *stream);
void Kinc_A1_SoundStreamSetLooping(Kinc_A1_SoundStream *stream, bool loop);
bool Kinc_A1_SoundStreamEnded(Kinc_A1_SoundStream *stream);
float Kinc_A1_SoundStreamLength(Kinc_A1_SoundStream *stream);
float Kinc_A1_SoundStreamPosition(Kinc_A1_SoundStream *stream);
void Kinc_A1_SoundStreamReset(Kinc_A1_SoundStream *stream);
float Kinc_A1_SoundStreamVolume(Kinc_A1_SoundStream *stream);
void Kinc_A1_SoundStreamSetVolume(Kinc_A1_SoundStream *stream, float value);
