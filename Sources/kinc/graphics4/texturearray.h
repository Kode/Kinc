#pragma once

#include <kinc/global.h>

#include "texture.h"

#include <kinc/backend/graphics4/texturearray.h>

/*! \file texturearray.h
    \brief Provides functions for creating and destroying texture-arrays.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_array {
	kinc_g4_texture_array_impl_t impl;
} kinc_g4_texture_array_t;

/// <summary>
/// Allocates and initializes a texture-array based on an array of images.
/// </summary>
/// <param name="array">The texture-array to initialize</param>
/// <param name="images">The images to assign to the texture-array</param>
/// <param name="count">The number of images</param>
KINC_FUNC void kinc_g4_texture_array_init(kinc_g4_texture_array_t *array, kinc_image_t *images, int count);

/// <summary>
/// Deallocates and destroys a texture-array
/// </summary>
/// <param name="array">The texture-array to destroy</param>
KINC_FUNC void kinc_g4_texture_array_destroy(kinc_g4_texture_array_t *array);

#ifdef __cplusplus
}
#endif
