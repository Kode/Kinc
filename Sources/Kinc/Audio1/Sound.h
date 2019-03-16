#pragma once

#include <Kinc/Audio2/Audio.h>

#include <stdint.h>

typedef struct {
	Kore_A2_BufferFormat format;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float my_volume;
} Kore_A1_Sound;

Kore_A1_Sound *Kore_A1_CreateSound(const char *filename);
void Kore_A1_DestroySound(Kore_A1_Sound *sound);
float Kore_A1_SoundVolume(Kore_A1_Sound *sound);
void Kore_A1_SoundSetVolume(Kore_A1_Sound *sound, float value);
