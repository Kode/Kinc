#include "pch.h"

#include "RenderTargetImpl.h"

#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/log.h>

extern kinc_g5_command_list_t commandList;

void kinc_g4_render_target_init(kinc_g4_render_target_t *render_target, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g4_render_target_format_t format, int stencilBufferBits,
                                      int contextId) {
	kinc_g5_render_target_init(&render_target->impl._renderTarget, width, height, depthBufferBits, antialiasing, (kinc_g5_render_target_format_t)format,
	                           stencilBufferBits, contextId);
	render_target->texWidth = render_target->width = width;
	render_target->texHeight = render_target->height = height;
	if (contextId >= 0) {
		kinc_g5_command_list_texture_to_render_target_barrier(&commandList, &render_target->impl._renderTarget);
		kinc_g5_command_list_clear(&commandList, &render_target->impl._renderTarget, KINC_G5_CLEAR_COLOR, 0, 0.0f, 0);
	}
}

void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *render_target, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {
	kinc_g5_render_target_init_cube(&render_target->impl._renderTarget, cubeMapSize, depthBufferBits, antialiasing, (kinc_g5_render_target_format_t)format,
	                                stencilBufferBits, contextId);
}

void kinc_g4_render_target_destroy(kinc_g4_render_target_t *render_target) {
	kinc_g5_render_target_destroy(&render_target->impl._renderTarget);
}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *render_target, kinc_g4_texture_unit_t unit) {
	kinc_g5_command_list_render_target_to_texture_barrier(&commandList, &render_target->impl._renderTarget);
	kinc_g5_render_target_use_color_as_texture(&render_target->impl._renderTarget, unit.impl._unit);
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *render_target, kinc_g4_texture_unit_t unit) {
	kinc_g5_render_target_use_depth_as_texture(&render_target->impl._renderTarget, unit.impl._unit);
}

void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *render_target, kinc_g4_render_target_t *source) {
	kinc_g5_render_target_set_depth_stencil_from(&render_target->impl._renderTarget, &source->impl._renderTarget);
}

void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *render_target, uint8_t *data) {
	kinc_g5_command_list_get_render_target_pixels(&commandList, &render_target->impl._renderTarget, data);
}

void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *render_target, int levels) {}
