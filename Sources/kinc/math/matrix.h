#pragma once

#include "vector.h"

/*! \file matrix.h
    \brief Provides basic matrix types and a few function which create transformation-matrices. This only provides functionality which is needed elsewhere in
   Kinc - if you need more, look up how transformation-matrices work and add some functions to your own project. Alternatively the Kore/C++-API also provides a
   more complete matrix-API.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_matrix3x3 {
	float m[3 * 3];
} kinc_matrix3x3_t;

KINC_FUNC float kinc_matrix3x3_get(kinc_matrix3x3_t *matrix, int x, int y);
KINC_FUNC void kinc_matrix3x3_set(kinc_matrix3x3_t *matrix, int x, int y, float value);
KINC_FUNC void kinc_matrix3x3_transpose(kinc_matrix3x3_t *matrix);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_identity(void);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_rotation_x(float alpha);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_rotation_y(float alpha);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_rotation_z(float alpha);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_translation(float x, float y);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x3_multiply(kinc_matrix3x3_t *a, kinc_matrix3x3_t *b);
KINC_FUNC kinc_vector3_t kinc_matrix3x3_multiply_vector(kinc_matrix3x3_t *a, kinc_vector3_t b);

typedef struct kinc_matrix4x4 {
	float m[4 * 4];
} kinc_matrix4x4_t;

KINC_FUNC float kinc_matrix4x4_get(kinc_matrix4x4_t *matrix, int x, int y);
KINC_FUNC void kinc_matrix4x4_set(kinc_matrix4x4_t *matrix, int x, int y, float value);
KINC_FUNC void kinc_matrix4x4_transpose(kinc_matrix4x4_t *matrix);
KINC_FUNC kinc_matrix4x4_t kinc_matrix4x4_multiply(kinc_matrix4x4_t *a, kinc_matrix4x4_t *b);

#ifdef __cplusplus
}
#endif
