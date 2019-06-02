#include "pch.h"

#include "Graphics.h"

#include <kinc/graphics4/rendertarget.h>

using namespace Kore;
using namespace Kore::Graphics4;

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	kinc_g4_render_target_init(&kincRenderTarget, width, height, depthBufferBits, antialiasing, (kinc_g4_render_target_format_t)format, stencilBufferBits,
	                           contextId);
}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	kinc_g4_render_target_init_cube(&kincRenderTarget, cubeMapSize, depthBufferBits, antialiasing, (kinc_g4_render_target_format_t)format, stencilBufferBits,
	                                contextId);
}

RenderTarget::~RenderTarget() {
	kinc_g4_render_target_destroy(&kincRenderTarget);
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	kinc_g4_render_target_use_color_as_texture(&kincRenderTarget, unit.kincUnit);
}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {
	kinc_g4_render_target_use_depth_as_texture(&kincRenderTarget, unit.kincUnit);
}

void RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	kinc_g4_render_target_set_depth_stencil_from(&kincRenderTarget, &source->kincRenderTarget);
}

void RenderTarget::getPixels(u8* data) {
	kinc_g4_render_target_get_pixels(&kincRenderTarget, data);
}

void RenderTarget::generateMipmaps(int levels) {
	kinc_g4_render_target_generate_mipmaps(&kincRenderTarget, levels);
}
