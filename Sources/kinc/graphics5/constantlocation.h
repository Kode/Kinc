#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_constant_location {
	ConstantLocation5Impl impl;
} kinc_g5_constant_location_t;

#ifdef __cplusplus
}
#endif
