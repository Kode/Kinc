#include "pch.h"

#include <Kinc/Graphics4/RenderTarget.h>
#include <Kinc/Graphics5/RenderTarget.h>
#include <Kinc/Log.h>

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {
	renderTarget->texWidth = renderTarget->width = width;
	renderTarget->texHeight = renderTarget->height = height;
}

void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {}

void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget) {}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {}

void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source) {}

void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data) {}

void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels) {}
