#pragma once

#include "pch.h"

#include "textureunit.h"

#include <kinc/backend/graphics5/rendertarget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g5_render_target_format {
	KINC_G5_RENDER_TARGET_FORMAT_32BIT,
	KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_16BIT_DEPTH,
	KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED,
	KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT
} kinc_g5_render_target_format_t;

typedef struct kinc_g5_render_target {
	int width;
	int height;
	int texWidth;
	int texHeight;
	int contextId;
	bool isCubeMap;
	bool isDepthAttachment;
	RenderTarget5Impl impl;
} kinc_g5_render_target_t;

KINC_FUNC void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, int depthBufferBits, bool antialiasing,
                                          kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId);
KINC_FUNC void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *target, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                               kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId);
KINC_FUNC void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target);
KINC_FUNC void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit);
KINC_FUNC void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit);
KINC_FUNC void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source);

#ifdef __cplusplus
}
#endif
