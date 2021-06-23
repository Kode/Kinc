#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/texture.h>

/*! \file textureunit.h
    \brief Provides a texture-unit-struct which is used for setting textures in a shader.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_texture_unit {
	TextureUnit5Impl impl;
} kinc_g5_texture_unit_t;

#ifdef __cplusplus
}
#endif
