#pragma once

#include "Texture.h"

#include <Kore/TextureArrayImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_array {
	Kinc_G4_TextureArrayImpl impl;
} kinc_g4_texture_array_t;

void kinc_g4_texture_array_init(kinc_g4_texture_array_t *array, kinc_image_t **textures, int count);

#ifdef __cplusplus
}
#endif
