#pragma once

#include <Kore/VertexBufferImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KINC_G4_VERTEX_DATA_NONE,
	KINC_G4_VERTEX_DATA_FLOAT1,
	KINC_G4_VERTEX_DATA_FLOAT2,
	KINC_G4_VERTEX_DATA_FLOAT3,
	KINC_G4_VERTEX_DATA_FLOAT4,
	KINC_G4_VERTEX_DATA_FLOAT4X4,
	KINC_G4_VERTEX_DATA_SHORT2_NORM,
	KINC_G4_VERTEX_DATA_SHORT4_NORM,
	KINC_G4_VERTEX_DATA_COLOR
} Kinc_G4_VertexData;

typedef struct {
	const char *name;
	Kinc_G4_VertexData data;
} Kinc_G4_VertexElement;

void Kinc_G4_VertexElement_Create(Kinc_G4_VertexElement *element, const char *name, Kinc_G4_VertexData data);

#define KINC_G4_MAX_VERTEX_ELEMENTS 16

typedef struct {
	Kinc_G4_VertexElement elements[KINC_G4_MAX_VERTEX_ELEMENTS];
	int size;
	bool instanced;
} Kinc_G4_VertexStructure;

void Kinc_G4_VertexStructure_Create(Kinc_G4_VertexStructure *structure);

void Kinc_G4_VertexStructure_Add(Kinc_G4_VertexStructure *structure, const char *name, Kinc_G4_VertexData data);

typedef struct {
	Kinc_G4_VertexBufferImpl impl;
} Kinc_G4_VertexBuffer;

typedef enum {
	KINC_G4_USAGE_STATIC,
	KINC_G4_USAGE_DYNAMIC,
	KINC_G4_USAGE_READABLE
} Kinc_G4_Usage;

void Kinc_G4_VertexBuffer_Create(Kinc_G4_VertexBuffer *buffer, int count, Kinc_G4_VertexStructure *structure, Kinc_G4_Usage usage, int instance_data_step_rate);
void Kinc_G4_VertexBuffer_Destroy(Kinc_G4_VertexBuffer *buffer);
float *Kinc_G4_VertexBuffer_LockAll(Kinc_G4_VertexBuffer *buffer);
float *Kinc_G4_VertexBuffer_Lock(Kinc_G4_VertexBuffer *buffer, int start, int count);
void Kinc_G4_VertexBuffer_UnlockAll(Kinc_G4_VertexBuffer *buffer);
void Kinc_G4_VertexBuffer_Unlock(Kinc_G4_VertexBuffer *buffer, int count);
int Kinc_G4_VertexBuffer_Count(Kinc_G4_VertexBuffer *buffer);
int Kinc_G4_VertexBuffer_Stride(Kinc_G4_VertexBuffer *buffer);

int Kinc_Internal_G4_VertexBuffer_Set(Kinc_G4_VertexBuffer *buffer, int offset);

void Kinc_G4_SetVertexBuffers(Kinc_G4_VertexBuffer **buffers, int count);

#ifdef __cplusplus
}
#endif
