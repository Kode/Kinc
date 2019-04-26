#pragma once

#include "pch.h"

#include <Kore/ConstantBuffer5Impl.h>

#include <Kinc/Math/Matrix.h>
#include <Kinc/Math/Vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_constant_buffer {
	uint8_t *data;
} kinc_g5_constant_buffer_t;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer *buffer, int size);
void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer *buffer);
void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer *buffer);
void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer *buffer, int start, int count);
void kinc_g5_constant_buffer_unlock_all(kinc_g5_constant_buffer *buffer);
int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer *buffer);

void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer *buffer, int offset, bool value);
void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer *buffer, int offset, int value);
void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer *buffer, int offset, float value);
void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2);
void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2, float value3);
void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2, float value3, float value4);
void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer *buffer, int offset, float *values, int count);
void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer *buffer, int offset, Kinc_Matrix3x3 *value);
void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer *buffer, int offset, Kinc_Matrix4x4 *value);

#ifdef __cplusplus
}
#endif
