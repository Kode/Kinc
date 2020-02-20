#include "pch.h"

#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/math/core.h>
#include <kinc/system.h>
#include <kinc/log.h>

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapChain;

void kinc_g5_destroy(int windowId) {}

void kinc_internal_g5_resize(int window, int width, int height) {}

void kinc_g5_init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	newRenderTargetWidth = renderTargetWidth = kinc_width();
	newRenderTargetHeight = renderTargetHeight = kinc_height();

	device = emscripten_webgpu_get_device();
	queue = wgpuDeviceCreateQueue(device);

	WGPUSurfaceDescriptorFromHTMLCanvasId canvasDesc;
	memset(&canvasDesc, 0, sizeof(canvasDesc));
	canvasDesc.id = "canvas";

	WGPUSurfaceDescriptor surfDesc;
	memset(&surfDesc, 0, sizeof(surfDesc));
	surfDesc.nextInChain = &canvasDesc;
	WGPUInstance instance = 0;
	WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfDesc);

	WGPUSwapChainDescriptor scDesc;
	memset(&scDesc, 0, sizeof(scDesc));
	scDesc.usage = WGPUTextureUsage_OutputAttachment;
	scDesc.format = WGPUTextureFormat_BGRA8Unorm;
	scDesc.width = kinc_width();
	scDesc.height = kinc_height();
	scDesc.presentMode = WGPUPresentMode_VSync;
	swapChain = wgpuDeviceCreateSwapChain(device, surface, &scDesc);
}

void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

void kinc_g5_draw_indexed_vertices_instanced_from_to_from(int instanceCount, int start, int count, int vertex_offset) {}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {}

void kinc_g5_end(int window) {}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_flush() {}

void kinc_g5_set_texture_operation(kinc_g5_texture_operation_t operation, kinc_g5_texture_argument_t arg1, kinc_g5_texture_argument_t arg2) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}
bool kinc_g5_non_pow2_textures_qupported() {
	return true;
}

void kinc_g5_set_render_target_face(kinc_g5_render_target_t *texture, int face) {}

void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	
}

void kinc_g5_set_image_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

bool kinc_g5_init_occlusion_query(unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_delete_occlusion_query(unsigned occlusionQuery) {}

void kinc_g5_render_occlusion_query(unsigned occlusionQuery, int triangles) {}

bool kinc_g5_are_query_results_available(unsigned occlusionQuery) {
	return false;
}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount) {}
