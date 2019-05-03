#include "pch.h"

#include "Matrix.h"

float kinc_matrix3x3_get(Kinc_Matrix3x3* matrix, int x, int y) {
	return matrix->m[y * 3 + x];
}

void kinc_matrix3x3_transpose(Kinc_Matrix3x3* matrix) {
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			float f = matrix->m[y * 3 + x];
			matrix->m[y * 3 + x] = matrix->m[x * 3 + y];
			matrix->m[x * 3 + y] = f;
		}
	}
}

float kinc_matrix4x4_get(Kinc_Matrix4x4* matrix, int x, int y) {
	return matrix->m[y * 4 + x];
}

void kinc_matrix4x4_transpose(Kinc_Matrix4x4* matrix) {
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			float f = matrix->m[y * 4 + x];
			matrix->m[y * 4 + x] = matrix->m[x * 4 + y];
			matrix->m[x * 4 + y] = f;
		}
	}
}
