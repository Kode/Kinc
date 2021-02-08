#pragma once

#include <kinc/backend/graphics4/shader.h>
#include <kinc/backend/graphics4/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_unit {
	kinc_g4_texture_unit_impl_t impl;
} kinc_g4_texture_unit_t;

#ifdef __cplusplus
}
#endif
