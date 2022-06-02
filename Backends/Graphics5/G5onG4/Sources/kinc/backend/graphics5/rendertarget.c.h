#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics5/rendertarget.h>
#include <kinc/log.h>

void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *renderTarget, int width, int height, kinc_g5_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	renderTarget->texWidth = renderTarget->width = width;
	renderTarget->texHeight = renderTarget->height = height;
	kinc_g4_render_target_init_with_multisampling(&renderTarget->impl, width, height, (kinc_g4_render_target_format_t)format, depthBufferBits, stencilBufferBits, samples_per_pixel);
}

void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                               int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
                                                                
                                                               }

void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	target->texWidth = target->width = cubeMapSize;
	target->texHeight = target->height = cubeMapSize;
    target->isCubeMap = true;
	kinc_g4_render_target_init_cube_with_multisampling(&target->impl, cubeMapSize, (kinc_g4_render_target_format_t)format, depthBufferBits,
	                                              stencilBufferBits, samples_per_pixel);
                                                        }

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {
    kinc_g4_render_target_destroy(&renderTarget->impl.target);
}

void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *renderTarget, kinc_g5_texture_unit_t unit) {
    kinc_g4_render_target_use_color_as_texture(&renderTarget->impl.target, unit.impl.unit);
}

void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *renderTarget, kinc_g5_texture_unit_t unit) {
    kinc_g4_render_target_use_depth_as_texture(&renderTarget->impl.target, unit.impl.unit);
}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {
    kinc_g4_render_target_set_depth_stencil_from(&renderTarget->impl.target, &source->impl.target);
}

// void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {
//     kinc_g4_render_target_get_pixels(&renderTarget->impl, data);
// }

// void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels) {
//     kinc_g4_render_target_generate_mipmaps(&renderTarget->impl, levels);
// }
