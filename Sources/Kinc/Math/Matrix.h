#pragma once

#include "Core.h"
#include "Vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float m[3 * 3];
} Kinc_Matrix3x3;

typedef struct {
	float m[3 * 3];
} Kinc_Matrix4x4;

#ifdef __cplusplus
}
#endif
