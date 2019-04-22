#pragma once

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

typedef struct _Kinc_G4_VertexStructure {
	Kinc_G4_VertexElement elements[KINC_G4_MAX_VERTEX_ELEMENTS];
	int size;
	bool instanced;
} Kinc_G4_VertexStructure;

void Kinc_G4_VertexStructure_Create(Kinc_G4_VertexStructure *structure);

void Kinc_G4_VertexStructure_Add(Kinc_G4_VertexStructure *structure, const char *name, Kinc_G4_VertexData data);

#ifdef __cplusplus
}
#endif
