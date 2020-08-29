#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g4_vertex_data {
	KINC_G4_VERTEX_DATA_NONE,
	KINC_G4_VERTEX_DATA_FLOAT1,
	KINC_G4_VERTEX_DATA_FLOAT2,
	KINC_G4_VERTEX_DATA_FLOAT3,
	KINC_G4_VERTEX_DATA_FLOAT4,
	KINC_G4_VERTEX_DATA_FLOAT4X4,
	KINC_G4_VERTEX_DATA_SHORT2_NORM,
	KINC_G4_VERTEX_DATA_SHORT4_NORM,
	KINC_G4_VERTEX_DATA_COLOR
} kinc_g4_vertex_data_t;

typedef struct kinc_g4_vertex_element {
	const char *name;
	kinc_g4_vertex_data_t data;
} kinc_g4_vertex_element_t;

KINC_FUNC void kinc_g4_vertex_element_init(kinc_g4_vertex_element_t *element, const char *name, kinc_g4_vertex_data_t data);

#define KINC_G4_MAX_VERTEX_ELEMENTS 16

typedef struct kinc_g4_vertex_structure {
	kinc_g4_vertex_element_t elements[KINC_G4_MAX_VERTEX_ELEMENTS];
	int size;
	bool instanced;
} kinc_g4_vertex_structure_t;

KINC_FUNC void kinc_g4_vertex_structure_init(kinc_g4_vertex_structure_t *structure);

KINC_FUNC void kinc_g4_vertex_structure_add(kinc_g4_vertex_structure_t *structure, const char *name, kinc_g4_vertex_data_t data);

#ifdef __cplusplus
}
#endif
