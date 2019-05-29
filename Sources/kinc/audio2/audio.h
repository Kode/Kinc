#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int channels;
	int samples_per_second;
	int bits_per_sample;
} Kinc_A2_BufferFormat;

typedef struct {
	Kinc_A2_BufferFormat format;
	uint8_t *data;
	int data_size;
	int read_location;
	int write_location;
} Kinc_A2_Buffer;

void Kinc_A2_Init();
void Kinc_A2_SetCallback(void (*Kinc_A2_audio_callback)(Kinc_A2_Buffer *buffer, int samples));
extern int Kinc_A2_SamplesPerSecond;
void Kinc_A2_Update();
void Kinc_A2_Shutdown();

#ifdef __cplusplus
}
#endif
