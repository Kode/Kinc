#pragma once

#include "core.h"

/*! \file vector.h
    \brief Provides basic vector types.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_vector2 {
	float x;
	float y;
} kinc_vector2_t;

typedef struct kinc_vector3 {
	float x;
	float y;
	float z;
} kinc_vector3_t;

typedef struct kinc_vector4 {
	float x;
	float y;
	float z;
	float w;
} kinc_vector4_t;

typedef struct kope_float2 {
	float x;
	float y;
} kope_float2;

typedef struct kope_float3 {
	float x;
	float y;
	float z;
} kope_float3;

typedef struct kope_float4 {
	float x;
	float y;
	float z;
	float w;
} kope_float4;

typedef struct kope_int2 {
	int32_t x;
	int32_t y;
} kope_int2;

typedef struct kope_int3 {
	int32_t x;
	int32_t y;
	int32_t z;
} kope_int3;

typedef struct kope_int4 {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;
} kope_int4;

typedef struct kope_uint2 {
	uint32_t x;
	uint32_t y;
} kope_uint2;

typedef struct kope_uint3 {
	uint32_t x;
	uint32_t y;
	uint32_t z;
} kope_uint3;

typedef struct kope_uint4 {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
} kope_uint4;

#ifdef __cplusplus
}
#endif
