#pragma once

#include "pch.h"

#include <Kore/ConstantBuffer5Impl.h>

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_constant_buffer {
	uint8_t *data;
	ConstantBuffer5Impl impl;
} kinc_g5_constant_buffer_t;

KINC_FUNC void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size);
KINC_FUNC void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer);
KINC_FUNC void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer);
KINC_FUNC void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count);
KINC_FUNC void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer);
KINC_FUNC int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer);

KINC_FUNC void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value);
KINC_FUNC void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value);
KINC_FUNC void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value);
KINC_FUNC void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2);
KINC_FUNC void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3);
KINC_FUNC void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4);
KINC_FUNC void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count);
KINC_FUNC void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value);
KINC_FUNC void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value);

KINC_FUNC extern bool kinc_g5_transposeMat3;
KINC_FUNC extern bool kinc_g5_transposeMat4;

#ifdef __cplusplus
}
#endif
