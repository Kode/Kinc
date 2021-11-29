#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file vertexstructure.h
    \brief Provides functions for setting up the structure of vertices in a vertex-buffer.
*/

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
	KINC_G4_VERTEX_DATA_BYTE1,
	KINC_G4_VERTEX_DATA_UNSIGNED_BYTE1,
	KINC_G4_VERTEX_DATA_NORMALIZED_BYTE1,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_BYTE1,
	KINC_G4_VERTEX_DATA_BYTE2,
	KINC_G4_VERTEX_DATA_UNSIGNED_BYTE2,
	KINC_G4_VERTEX_DATA_NORMALIZED_BYTE2,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_BYTE2,
	KINC_G4_VERTEX_DATA_BYTE4,
	KINC_G4_VERTEX_DATA_UNSIGNED_BYTE4,
	KINC_G4_VERTEX_DATA_NORMALIZED_BYTE4,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_BYTE4,
	KINC_G4_VERTEX_DATA_SHORT1,
	KINC_G4_VERTEX_DATA_UNSIGNED_SHORT1,
	KINC_G4_VERTEX_DATA_NORMALIZED_SHORT1,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_SHORT1,
	KINC_G4_VERTEX_DATA_SHORT2,
	KINC_G4_VERTEX_DATA_UNSIGNED_SHORT2,
	KINC_G4_VERTEX_DATA_NORMALIZED_SHORT2,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_SHORT2,
	KINC_G4_VERTEX_DATA_SHORT4,
	KINC_G4_VERTEX_DATA_UNSIGNED_SHORT4,
	KINC_G4_VERTEX_DATA_NORMALIZED_SHORT4,
	KINC_G4_VERTEX_DATA_NORMALIZED_UNSIGNED_SHORT4,
	KINC_G4_VERTEX_DATA_INT1,
	KINC_G4_VERTEX_DATA_UNSIGNED_INT1,
	KINC_G4_VERTEX_DATA_INT2,
	KINC_G4_VERTEX_DATA_UNSIGNED_INT2,
	KINC_G4_VERTEX_DATA_INT3,
	KINC_G4_VERTEX_DATA_UNSIGNED_INT3,
	KINC_G4_VERTEX_DATA_INT4,
	KINC_G4_VERTEX_DATA_UNSIGNED_INT4
} kinc_g4_vertex_data_t;

typedef struct kinc_g4_vertex_element {
	const char *name;
	kinc_g4_vertex_data_t data;
} kinc_g4_vertex_element_t;

#define KINC_G4_MAX_VERTEX_ELEMENTS 16

typedef struct kinc_g4_vertex_structure {
	kinc_g4_vertex_element_t elements[KINC_G4_MAX_VERTEX_ELEMENTS];
	int size;
	bool instanced;
} kinc_g4_vertex_structure_t;

/// <summary>
/// Initializes a vertex-structure.
/// </summary>
/// <param name="structure">The structure to initialize</param>
/// <returns></returns>
KINC_FUNC void kinc_g4_vertex_structure_init(kinc_g4_vertex_structure_t *structure);

/// <summary>
/// Adds an element to a vertex-structure.
/// </summary>
/// <param name="structure">The structure to add an element to</param>
/// <param name="name">The name to use for the new element</param>
/// <param name="data">The type of data to assign for the new element</param>
/// <returns></returns>
KINC_FUNC void kinc_g4_vertex_structure_add(kinc_g4_vertex_structure_t *structure, const char *name, kinc_g4_vertex_data_t data);

#ifdef __cplusplus
}
#endif
