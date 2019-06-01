#pragma once

#include "texture.h"

#include <Kore/TextureArrayImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_array {
	kinc_g4_texture_array_impl_t impl;
} kinc_g4_texture_array_t;

void kinc_g4_texture_array_init(kinc_g4_texture_array_t *array, kinc_image_t **textures, int count);

#ifdef __cplusplus
}
#endif
