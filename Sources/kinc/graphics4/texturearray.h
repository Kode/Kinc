#pragma once

#include <kinc/global.h>

#include "texture.h"

#include <kinc/backend/graphics4/texturearray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_array {
	kinc_g4_texture_array_impl_t impl;
} kinc_g4_texture_array_t;

KINC_FUNC void kinc_g4_texture_array_init(kinc_g4_texture_array_t *array, kinc_image_t *textures, int count);
KINC_FUNC void kinc_g4_texture_array_destroy(kinc_g4_texture_array_t *array);

#ifdef __cplusplus
}
#endif
