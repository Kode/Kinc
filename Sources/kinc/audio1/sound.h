#pragma once

#include <kinc/audio2/audio.h>

#include <stdint.h>

typedef struct {
	Kinc_A2_BufferFormat format;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float my_volume;
} Kinc_A1_Sound;

Kinc_A1_Sound *Kinc_A1_CreateSound(const char *filename);
void Kinc_A1_DestroySound(Kinc_A1_Sound *sound);
float Kinc_A1_SoundVolume(Kinc_A1_Sound *sound);
void Kinc_A1_SoundSetVolume(Kinc_A1_Sound *sound, float value);
