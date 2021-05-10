#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/rendertarget.h>

#include "textureunit.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g4_render_target_format {
	KINC_G4_RENDER_TARGET_FORMAT_32BIT,
	KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH,
	KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT
} kinc_g4_render_target_format_t;

typedef struct kinc_g4_render_target {
	int width;
	int height;
	int texWidth;
	int texHeight;
	int contextId;
	bool isCubeMap;
	bool isDepthAttachment;

	kinc_g4_render_target_impl_t impl;
} kinc_g4_render_target_t;

KINC_FUNC void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, int depthBufferBits, bool antialiasing,
                                          kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId);

KINC_FUNC void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                               kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId);

KINC_FUNC void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget);

KINC_FUNC void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit);
KINC_FUNC void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit);
KINC_FUNC void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source);
KINC_FUNC void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data);
KINC_FUNC void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels);

#ifdef __cplusplus
}
#endif
