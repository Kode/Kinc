#pragma once

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float x;
	float y;
} kinc_vector2_t;

typedef struct {
	float x;
	float y;
	float z;
} kinc_vector3_t;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} kinc_vector4_t;

#ifdef __cplusplus
}
#endif
