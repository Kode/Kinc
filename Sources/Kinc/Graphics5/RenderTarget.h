#pragma once

#include "pch.h"

#include "TextureUnit.h"

#include <Kore/RenderTarget5Impl.h>

#include <Kinc/Graphics4/RenderTarget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef kinc_g4_render_target_format_t kinc_g5_render_target_format_t;

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

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId);
void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *target, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId);
void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target);
void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit);
void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit);
void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source);

#ifdef __cplusplus
}
#endif
