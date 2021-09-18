#pragma once

#include <kinc/graphics5/rendertarget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// RenderTargetImpl(int width, int height, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits,
	//	                int contextId);
	// RenderTargetImpl(int cubeMapSize, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId);
	kinc_g5_render_target_t _renderTarget;
} kinc_g4_render_target_impl_t;

#ifdef __cplusplus
}
#endif
