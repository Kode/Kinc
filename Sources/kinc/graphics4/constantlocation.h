#pragma once

#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/backend/graphics4/shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_constant_location {
	kinc_g4_constant_location_impl_t impl;
} kinc_g4_constant_location_t;

#ifdef __cplusplus
}
#endif
