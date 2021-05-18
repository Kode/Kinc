#pragma once

#include "core.h"

/*! \file quaternion.h
    \brief Provides a basic quaternion type.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float x;
	float y;
	float z;
	float w;
} kinc_quaternion_t;

#ifdef __cplusplus
}
#endif
