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
	KINC_G4_VERTEX_DATA_F32_1,
	KINC_G4_VERTEX_DATA_F32_2,
	KINC_G4_VERTEX_DATA_F32_3,
	KINC_G4_VERTEX_DATA_F32_4,
	KINC_G4_VERTEX_DATA_F32_4X4,
	KINC_G4_VERTEX_DATA_I8_1,
	KINC_G4_VERTEX_DATA_U8_1,
	KINC_G4_VERTEX_DATA_NORMALIZED_I8_1,
	KINC_G4_VERTEX_DATA_NORMALIZED_U8_1,
	KINC_G4_VERTEX_DATA_I8_2,
	KINC_G4_VERTEX_DATA_U8_2,
	KINC_G4_VERTEX_DATA_NORMALIZED_I8_2,
	KINC_G4_VERTEX_DATA_NORMALIZED_U8_2,
	KINC_G4_VERTEX_DATA_I8_4,
	KINC_G4_VERTEX_DATA_U8_4,
	KINC_G4_VERTEX_DATA_NORMALIZED_I8_4,
	KINC_G4_VERTEX_DATA_NORMALIZED_U8_4,
	KINC_G4_VERTEX_DATA_I16_1,
	KINC_G4_VERTEX_DATA_U16_1,
	KINC_G4_VERTEX_DATA_NORMALIZED_I16_1,
	KINC_G4_VERTEX_DATA_NORMALIZED_U16_1,
	KINC_G4_VERTEX_DATA_I16_2,
	KINC_G4_VERTEX_DATA_U16_2,
	KINC_G4_VERTEX_DATA_NORMALIZED_I16_2,
	KINC_G4_VERTEX_DATA_NORMALIZED_U16_2,
	KINC_G4_VERTEX_DATA_I16_4,
	KINC_G4_VERTEX_DATA_U16_4,
	KINC_G4_VERTEX_DATA_NORMALIZED_I16_4,
	KINC_G4_VERTEX_DATA_NORMALIZED_U16_4,
	KINC_G4_VERTEX_DATA_I32_1,
	KINC_G4_VERTEX_DATA_U32_1,
	KINC_G4_VERTEX_DATA_I32_2,
	KINC_G4_VERTEX_DATA_U32_2,
	KINC_G4_VERTEX_DATA_I32_3,
	KINC_G4_VERTEX_DATA_U32_3,
	KINC_G4_VERTEX_DATA_I32_4,
	KINC_G4_VERTEX_DATA_U32_4
} kinc_g4_vertex_data_t;

static inline int kinc_g4_vertex_data_size(kinc_g4_vertex_data_t data) {
	switch (data) {
	default:
	case KINC_G4_VERTEX_DATA_NONE:
		return 0;
	case KINC_G4_VERTEX_DATA_F32_1:
		return 1 * 4;
	case KINC_G4_VERTEX_DATA_F32_2:
		return 2 * 4;
	case KINC_G4_VERTEX_DATA_F32_3:
		return 3 * 4;
	case KINC_G4_VERTEX_DATA_F32_4:
		return 4 * 4;
	case KINC_G4_VERTEX_DATA_F32_4X4:
		return 4 * 4 * 4;
	case KINC_G4_VERTEX_DATA_I8_1:
	case KINC_G4_VERTEX_DATA_U8_1:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I8_1:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U8_1:
		return 1 * 1;
	case KINC_G4_VERTEX_DATA_I8_2:
	case KINC_G4_VERTEX_DATA_U8_2:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I8_2:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U8_2:
		return 2 * 1;
	case KINC_G4_VERTEX_DATA_I8_4:
	case KINC_G4_VERTEX_DATA_U8_4:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I8_4:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U8_4:
		return 4 * 1;
	case KINC_G4_VERTEX_DATA_I16_1:
	case KINC_G4_VERTEX_DATA_U16_1:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I16_1:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U16_1:
		return 1 * 2;
	case KINC_G4_VERTEX_DATA_I16_2:
	case KINC_G4_VERTEX_DATA_U16_2:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I16_2:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U16_2:
		return 2 * 2;
	case KINC_G4_VERTEX_DATA_I16_4:
	case KINC_G4_VERTEX_DATA_U16_4:
	case KINC_G4_VERTEX_DATA_NORMALIZED_I16_4:
	case KINC_G4_VERTEX_DATA_NORMALIZED_U16_4:
		return 4 * 2;
	case KINC_G4_VERTEX_DATA_I32_1:
	case KINC_G4_VERTEX_DATA_U32_1:
		return 1 * 4;
	case KINC_G4_VERTEX_DATA_I32_2:
	case KINC_G4_VERTEX_DATA_U32_2:
		return 2 * 4;
	case KINC_G4_VERTEX_DATA_I32_3:
	case KINC_G4_VERTEX_DATA_U32_3:
		return 3 * 4;
	case KINC_G4_VERTEX_DATA_I32_4:
	case KINC_G4_VERTEX_DATA_U32_4:
		return 4 * 4;
	}
}

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
