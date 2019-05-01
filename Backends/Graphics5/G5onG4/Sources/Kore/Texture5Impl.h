#pragma once

#include <Kinc/Graphics4/Texture.h>
#include <Kinc/Graphics4/TextureUnit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g4_texture_unit_t unit;
} TextureUnit5Impl;

typedef struct {
	kinc_g4_texture_t texture;
} Texture5Impl;

#ifdef __cplusplus
}
#endif
