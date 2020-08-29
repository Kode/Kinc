#pragma once

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
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x_rotation_x(float alpha);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x_rotation_y(float alpha);
KINC_FUNC kinc_matrix3x3_t kinc_matrix3x_rotation_z(float alpha);

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
