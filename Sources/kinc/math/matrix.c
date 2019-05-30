#include "pch.h"

#include <kinc/math/core.h>
#include <kinc/math/matrix.h>

float kinc_matrix3x3_get(kinc_matrix3x3_t *matrix, int x, int y) {
	return matrix->m[y * 3 + x];
}

void kinc_matrix3x3_set(kinc_matrix3x3_t* matrix, int x, int y, float value) {
	matrix->m[y * 3 + x] = value;
}

void kinc_matrix3x3_transpose(kinc_matrix3x3_t *matrix) {
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			float f = matrix->m[y * 3 + x];
			matrix->m[y * 3 + x] = matrix->m[x * 3 + y];
			matrix->m[x * 3 + y] = f;
		}
	}
}

kinc_matrix3x3_t kinc_matrix3x3_identity(void) {
	kinc_matrix3x3_t m;
	for (unsigned x = 0; x < 3; ++x) {
		kinc_matrix3x3_set(&m, x, x, 1.0f);
	}
	return m;
}

kinc_matrix3x3_t kinc_matrix3x_rotation_x(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = kinc_cos(alpha);
	float sa = kinc_sin(alpha);
	kinc_matrix3x3_set(&m, 1, 1,  ca);
	kinc_matrix3x3_set(&m, 1, 2, -sa);
	kinc_matrix3x3_set(&m, 2, 1,  sa);
	kinc_matrix3x3_set(&m, 2, 2,  ca);
	return m;
}

kinc_matrix3x3_t kinc_matrix3x_rotation_y(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = kinc_cos(alpha);
	float sa = kinc_sin(alpha);
	kinc_matrix3x3_set(&m, 0, 0,  ca);
	kinc_matrix3x3_set(&m, 0, 2,  sa);
	kinc_matrix3x3_set(&m, 2, 0, -sa);
	kinc_matrix3x3_set(&m, 2, 2,  ca);
	return m;
}

kinc_matrix3x3_t kinc_matrix3x_rotation_z(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = kinc_cos(alpha);
	float sa = kinc_sin(alpha);
	kinc_matrix3x3_set(&m, 0, 0,  ca);
	kinc_matrix3x3_set(&m, 0, 1, -sa);
	kinc_matrix3x3_set(&m, 1, 0,  sa);
	kinc_matrix3x3_set(&m, 1, 1,  ca);
	return m;
}

float kinc_matrix4x4_get(kinc_matrix4x4_t *matrix, int x, int y) {
	return matrix->m[y * 4 + x];
}

void kinc_matrix4x4_transpose(kinc_matrix4x4_t *matrix) {
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			float f = matrix->m[y * 4 + x];
			matrix->m[y * 4 + x] = matrix->m[x * 4 + y];
			matrix->m[x * 4 + y] = f;
		}
	}
}
