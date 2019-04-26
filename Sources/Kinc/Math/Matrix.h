#pragma once

#include "Core.h"
#include "Vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float m[3 * 3];
} Kinc_Matrix3x3;

float kinc_matrix3x3_get(Kinc_Matrix3x3 *matrix, int x, int y);
void kinc_matrix3x3_transpose(Kinc_Matrix3x3 *matrix);

typedef struct {
	float m[3 * 3];
} Kinc_Matrix4x4;

float kinc_matrix4x4_get(Kinc_Matrix4x4 *matrix, int x, int y);
void kinc_matrix4x4_transpose(Kinc_Matrix4x4 *matrix);

#ifdef __cplusplus
}
#endif
