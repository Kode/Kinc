#pragma once

#include <kinc/backend/pipeline.h>
#include <kinc/backend/shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_constant_location {
	kinc_g4_constant_location_impl_t impl;
} kinc_g4_constant_location_t;

#ifdef __cplusplus
}
#endif
