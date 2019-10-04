#include "pch.h"

#include "Metal.h"
#include "VertexBuffer5Impl.h"

#include <kinc/math/core.h>
#include <kinc/system.h>
#include <kinc/window.h>

#import <Metal/Metal.h>

#include <cstdio>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

extern "C" {
	int renderTargetWidth;
	int renderTargetHeight;
	int newRenderTargetWidth;
	int newRenderTargetHeight;
}

namespace {
	// bool fullscreen;
	// TextureFilter minFilters[32];
	// MipmapFilter mipFilters[32];
	// int originalFramebuffer;
}

void kinc_g5_destroy(int window) {

}

extern "C" void kinc_internal_resize(int window, int width, int height) {

}

void kinc_g5_init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	// System::createWindow();

}

void kinc_g5_flush() {}

// void* Graphics::getControl() {
//	return nullptr;
//}

void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

void beginGL();

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
	beginGL();
}

void kinc_g5_end(int window) {

}

#ifdef KORE_MACOS
void swapBuffersMac(int windowId);
#endif

#ifdef KORE_IOS
void swapBuffersiOS();
#endif

bool kinc_g5_swap_buffers() {
#ifdef KORE_MACOS
	swapBuffersMac(0);
#endif
#ifdef KORE_IOS
	swapBuffersiOS();
#endif
	return true;
}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount);

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

void kinc_g5_set_texture_operation(kinc_g5_texture_operation_t operation, kinc_g5_texture_argument_t arg1, kinc_g5_texture_argument_t arg2) {}

void kinc_g5_set_render_target_face(kinc_g5_render_target_t *texture, int face) {}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}

bool kinc_g5_non_pow2_textures_qupported() {
	return true;
}

void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	kinc_g5_internal_texture_set(texture, unit.impl.index);
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

bool kinc_window_vsynced(int window) {
	return true;
}
