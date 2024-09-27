#ifndef KOPE_G5_TEXTURE_HEADER
#define KOPE_G5_TEXTURE_HEADER

#include <kope/global.h>

#include "api.h"
#include "textureformat.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/texture_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/texture_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_texture {
#ifdef KOPE_G5_VALIDATION
	kope_g5_texture_format validation_format;
#endif
	KOPE_G5_IMPL(texture);
} kope_g5_texture;

typedef enum kope_g5_texture_view_dimension {
	KOPE_G5_TEXTURE_VIEW_DIMENSION_1D,
	KOPE_G5_TEXTURE_VIEW_DIMENSION_2D,
	KOPE_G5_TEXTURE_VIEW_DIMENSION_2DARRAY,
	KOPE_G5_TEXTURE_VIEW_DIMENSION_CUBE,
	KOPE_G5_TEXTURE_VIEW_DIMENSION_CUBEARRAY,
	KOPE_G5_TEXTURE_VIEW_DIMENSION_3D
} kope_g5_texture_view_dimension;

typedef enum kope_g5_texture_aspect {
	KOPE_G5_IMAGE_COPY_ASPECT_ALL,
	KOPE_G5_IMAGE_COPY_ASPECT_DEPTH_ONLY,
	KOPE_G5_IMAGE_COPY_ASPECT_STENCIL_ONLY
} kope_g5_texture_aspect;

typedef struct kope_g5_texture_view {
	kope_g5_texture *texture;
	kope_g5_texture_format format;
	kope_g5_texture_view_dimension dimension;
	kope_g5_texture_aspect aspect;
	uint32_t base_mip_level;
	uint32_t mip_level_count;
	uint32_t base_array_layer;
	uint32_t array_layer_count;
} kope_g5_texture_view;

#ifdef __cplusplus
}
#endif

#endif
