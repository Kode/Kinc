#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int channels;
	int samples_per_second;
	int bits_per_sample;
} Kore_A2_BufferFormat;

typedef struct {
	Kore_A2_BufferFormat format;
	uint8_t *data;
	int data_size;
	int read_location;
	int write_location;
} Kore_A2_Buffer;

void Kore_A2_Init();
void Kore_A2_SetCallback(void (*Kore_A2_audio_callback)(Kore_A2_Buffer* buffer, int samples));
void Kore_A2_Update();
void Kore_A2_Shutdown();

#ifdef __cplusplus
}
#endif
