#pragma once

#include <kinc/global.h>

#include <kinc/image.h>

#include <kinc/backend/graphics6/texture.h>

/*! \file texture.h
    \brief Provides functions for setting up and using textures.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_texture_format {
	KINC_G6_TEXTURE_FORMAT_NONE,

	KINC_G6_TEXTURE_FORMAT_R8_UNORM,
	KINC_G6_TEXTURE_FORMAT_R8_SNORM,
	KINC_G6_TEXTURE_FORMAT_R8_UINT,
	KINC_G6_TEXTURE_FORMAT_R8_SINT,

	KINC_G6_TEXTURE_FORMAT_R16_UINT,
	KINC_G6_TEXTURE_FORMAT_R16_SINT,
	KINC_G6_TEXTURE_FORMAT_R16_FLOAT,
	KINC_G6_TEXTURE_FORMAT_RG8_UNORM,
	KINC_G6_TEXTURE_FORMAT_RG8_SNORM,
	KINC_G6_TEXTURE_FORMAT_RG8_UINT,
	KINC_G6_TEXTURE_FORMAT_RG8_SINT,

	KINC_G6_TEXTURE_FORMAT_R32_UINT,
	KINC_G6_TEXTURE_FORMAT_R32_SINT,
	KINC_G6_TEXTURE_FORMAT_R32_FLOAT,
	KINC_G6_TEXTURE_FORMAT_RG16_UINT,
	KINC_G6_TEXTURE_FORMAT_RG16_SINT,
	KINC_G6_TEXTURE_FORMAT_RG16_FLOAT,
	KINC_G6_TEXTURE_FORMAT_RGBA8_UNORM,
	KINC_G6_TEXTURE_FORMAT_RGBA8_UNORM_SRGB,
	KINC_G6_TEXTURE_FORMAT_RGBA8_SNORM,
	KINC_G6_TEXTURE_FORMAT_RGBA8_UINT,
	KINC_G6_TEXTURE_FORMAT_RGBA8_SINT,
	KINC_G6_TEXTURE_FORMAT_BGRA8_UNORM,
	KINC_G6_TEXTURE_FORMAT_BGRA8_UNORM_SRGB,

	KINC_G6_TEXTURE_FORMAT_RGB9E5_UFLOAT,
	KINC_G6_TEXTURE_FORMAT_RGB10A2_UNORM,
	KINC_G6_TEXTURE_FORMAT_RG11B10_UFLOAT,

	KINC_G6_TEXTURE_FORMAT_RG32_UINT,
	KINC_G6_TEXTURE_FORMAT_RG32_SINT,
	KINC_G6_TEXTURE_FORMAT_RG32_FLOAT,
	KINC_G6_TEXTURE_FORMAT_RGBA16_UINT,
	KINC_G6_TEXTURE_FORMAT_RGBA16_SINT,
	KINC_G6_TEXTURE_FORMAT_RGBA16_FLOAT,

	KINC_G6_TEXTURE_FORMAT_RGBA32_UINT,
	KINC_G6_TEXTURE_FORMAT_RGBA32_SINT,
	KINC_G6_TEXTURE_FORMAT_RGBA32_FLOAT,

	KINC_G6_TEXTURE_FORMAT_STENCIL8,
	KINC_G6_TEXTURE_FORMAT_DEPTH16_UNORM,
	KINC_G6_TEXTURE_FORMAT_DEPTH24_PLUS,
	KINC_G6_TEXTURE_FORMAT_DEPTH24_PLUS_STENCIL8,
	KINC_G6_TEXTURE_FORMAT_DEPTH32_FLOAT
} kinc_g6_texture_format_t;

typedef enum kinc_g6_texture_usage_bits {
	KINC_G6_TEXTURE_USAGE_COPY_SRC = 0x001,
	KINC_G6_TEXTURE_USAGE_COPY_DST = 0x002,
	KINC_G6_TEXTURE_USAGE_TEXTURE_BINDING = 0x004,
	KINC_G6_TEXTURE_USAGE_STORAGE_BINDING = 0x008,
	KINC_G6_TEXTURE_USAGE_RENDER_ATTACHMENT = 0x010
} kinc_g6_texture_usage_bits_t;

typedef uint32_t kinc_g6_texture_usage_t;

typedef enum kinc_g6_texture_dimension { KINC_G6_TEXTURE_DIMENSION_1D, KINC_G6_TEXTURE_DIMENSION_2D, KINC_G6_TEXTURE_DIMENSION_3D } kinc_g6_texture_dimension_t;

typedef struct kinc_g6_texture {
	kinc_g6_texture_impl_t impl;
} kinc_g6_texture_t;

typedef struct kinc_g6_texture_descriptor {
	int width;
	int height;
	int depth;
	int arrayLayers;
	int mipLevels;
	int sampleCount;
	kinc_g6_texture_dimension_t dimension;
	kinc_g6_texture_usage_t usage;
	kinc_g6_texture_format_t format;
} kinc_g6_texture_descriptor_t;

KINC_FUNC void kinc_g6_texture_init(kinc_g6_texture_t *texture, const kinc_g6_texture_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_texture_destroy(kinc_g6_texture_t *texture);

typedef struct kinc_g6_texture_view {
	kinc_g6_texture_view_impl_t impl;
} kinc_g6_texture_view_t;

typedef enum kinc_g6_texture_aspect {
	KINC_G6_TEXTURE_ASPECT_ALL = 0,
	KINC_G6_TEXTURE_ASPECT_STENCIL_ONLY = 1,
	KINC_G6_TEXTURE_ASPECT_DEPTH_ONLY = 2,
} kinc_g6_texture_aspect_t;

typedef enum kinc_g6_texture_view_dimension {
	KINC_G6_TEXTURE_VIEW_DIMENSION_1D = 0,
	KINC_G6_TEXTURE_VIEW_DIMENSION_2D = 1,
	KINC_G6_TEXTURE_VIEW_DIMENSION_3D = 2,
	KINC_G6_TEXTURE_VIEW_DIMENSION_CUBE = 3,
	KINC_G6_TEXTURE_VIEW_DIMENSION_1D_ARRAY = 4,
	KINC_G6_TEXTURE_VIEW_DIMENSION_2D_ARRAY = 5,
	KINC_G6_TEXTURE_VIEW_DIMENSION_CUBE_ARRAY = 6,
} kinc_g6_texture_view_dimension_t;

typedef struct kinc_g6_texture_view_descriptor {
	kinc_g6_texture_t *texture;
	kinc_g6_texture_format_t format;
	kinc_g6_texture_dimension_t dimension;
	kinc_g6_texture_aspect_t aspect;
	uint32_t base_mip_level;
	uint32_t mip_level_count;
	uint32_t base_array_layer;
	uint32_t array_layer_count;
} kinc_g6_texture_view_descriptor_t;

KINC_FUNC void kinc_g6_texture_view_init(kinc_g6_texture_view_t *view, const kinc_g6_texture_view_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_texture_view_destroy(kinc_g6_texture_view_t *view);

#ifdef __cplusplus
}
#endif