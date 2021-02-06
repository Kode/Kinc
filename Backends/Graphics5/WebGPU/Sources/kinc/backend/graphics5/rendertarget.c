#include "pch.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/log.h>

void kinc_g5_render_target_init(kinc_g5_render_target_t *renderTarget, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {
	
}

void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *renderTarget, kinc_g5_texture_unit_t unit) {}

void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *renderTarget, kinc_g5_texture_unit_t unit) {}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {}

void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {}

void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels) {}
