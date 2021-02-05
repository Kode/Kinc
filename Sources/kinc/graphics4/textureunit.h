#pragma once

#include <kinc/backend/shader.h>
#include <kinc/backend/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_unit {
	kinc_g4_texture_unit_impl_t impl;
} kinc_g4_texture_unit_t;

#ifdef __cplusplus
}
#endif
