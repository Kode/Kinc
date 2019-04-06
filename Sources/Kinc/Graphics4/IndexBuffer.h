#pragma once

#include <Kore/IndexBufferImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Kinc_G4_IndexBufferImpl impl;
} Kinc_G4_IndexBuffer;

void Kinc_G4_IndexBuffer_Create(Kinc_G4_IndexBuffer *buffer, int count);
void Kinc_G4_IndexBuffer_Destroy(Kinc_G4_IndexBuffer *buffer);
int *Kinc_G4_IndexBuffer_Lock(Kinc_G4_IndexBuffer *buffer);
void Kinc_G4_IndexBuffer_Unlock(Kinc_G4_IndexBuffer *buffer);
int Kinc_G4_IndexBuffer_Count(Kinc_G4_IndexBuffer *buffer);

void Kinc_Internal_G4_IndexBuffer_Set(Kinc_G4_IndexBuffer *buffer);

void Kinc_G4_SetIndexBuffer(Kinc_G4_IndexBuffer *buffer);

#ifdef __cplusplus
}
#endif
