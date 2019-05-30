#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_matrix3x3 {
	float m[3 * 3];
} kinc_matrix3x3_t;

float kinc_matrix3x3_get(kinc_matrix3x3_t *matrix, int x, int y);
void kinc_matrix3x3_set(kinc_matrix3x3_t *matrix, int x, int y, float value);
void kinc_matrix3x3_transpose(kinc_matrix3x3_t *matrix);
kinc_matrix3x3_t kinc_matrix3x3_identity(void);
kinc_matrix3x3_t kinc_matrix3x_rotation_x(float alpha);
kinc_matrix3x3_t kinc_matrix3x_rotation_y(float alpha);
kinc_matrix3x3_t kinc_matrix3x_rotation_z(float alpha);

typedef struct kinc_matrix4x4 {
	float m[3 * 3];
} kinc_matrix4x4_t;

float kinc_matrix4x4_get(kinc_matrix4x4_t *matrix, int x, int y);
void kinc_matrix4x4_transpose(kinc_matrix4x4_t *matrix);

#ifdef __cplusplus
}
#endif
