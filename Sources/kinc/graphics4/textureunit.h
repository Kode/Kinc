#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/shader.h>
#include <kinc/backend/graphics4/texture.h>

/*! \file textureunit.h
    \brief Provides a texture-unit-struct which is used for setting textures in a shader.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_unit {
	kinc_g4_texture_unit_impl_t impl;
} kinc_g4_texture_unit_t;

#ifdef __cplusplus
}
#endif
