#include "Metal.h"

#include <kinc/color.h>
#include <kinc/system.h>
#include <kinc/window.h>

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

id getMetalLayer(void);
id getMetalDevice(void);
id getMetalQueue(void);

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

id<CAMetalDrawable> drawable;
id<MTLCommandBuffer> commandBuffer;
id<MTLRenderCommandEncoder> commandEncoder;
id<MTLTexture> depthTexture;
int depthBits;
int stencilBits;

id getMetalEncoder(void) {
	return commandEncoder;
}

void kinc_g5_internal_destroy_window(int window) {}

void kinc_g5_internal_destroy() {}

void kinc_internal_resize(int window, int width, int height) {}

extern void kinc_internal_init_samplers(void);

void kinc_g5_internal_init() {}

void kinc_g5_internal_init_window(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	depthBits = depthBufferBits;
	stencilBits = stencilBufferBits;
	kinc_internal_init_samplers();
}

void kinc_g5_flush() {}

void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

bool kinc_internal_metal_has_depth = false;

bool kinc_internal_current_render_target_has_depth(void) {
	return kinc_internal_metal_has_depth;
}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
	CAMetalLayer *metalLayer = getMetalLayer();
	drawable = [metalLayer nextDrawable];

	kinc_internal_metal_has_depth = renderTarget->impl._depthTex != nil;

	if (depthBits > 0 && (depthTexture == nil || depthTexture.width != drawable.texture.width || depthTexture.height != drawable.texture.height)) {
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = drawable.texture.width;
		descriptor.height = drawable.texture.height;
		descriptor.depth = 1;
		descriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		descriptor.arrayLength = 1;
		descriptor.mipmapLevelCount = 1;
		descriptor.resourceOptions = MTLResourceStorageModePrivate;
		descriptor.usage = MTLTextureUsageRenderTarget;
		id<MTLDevice> device = getMetalDevice();
		depthTexture = [device newTextureWithDescriptor:descriptor];
	}

	id<MTLTexture> texture = drawable.texture;
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
	renderPassDescriptor.depthAttachment.clearDepth = 1;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = depthTexture;
	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
	renderPassDescriptor.stencilAttachment.texture = depthTexture;

	id<MTLCommandQueue> commandQueue = getMetalQueue();
	commandBuffer = [commandQueue commandBuffer];
	commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void kinc_g5_end(int window) {}

bool kinc_g5_swap_buffers() {
	if (commandBuffer != nil && commandEncoder != nil) {
		[commandEncoder endEncoding];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
	}
	commandBuffer = nil;
	commandEncoder = nil;

	return true;
}

extern bool kinc_internal_bilinear_filtering;

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	kinc_internal_bilinear_filtering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

int kinc_g5_max_bound_textures(void) {
	return 16;
}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount);

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	kinc_internal_bilinear_filtering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

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

void kinc_g5_internal_new_render_pass(kinc_g5_render_target_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth,
                                      int stencil) {
	[commandEncoder endEncoding];
	[commandBuffer commit];
	if (wait) {
		[commandBuffer waitUntilCompleted];
	}

	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	for (int i = 0; i < count; ++i) {
		renderPassDescriptor.colorAttachments[i].texture = renderTargets == NULL ? drawable.texture : (__bridge id<MTLTexture>)renderTargets[i]->impl._tex;
		if (clear_flags & KINC_G5_CLEAR_COLOR) {
			float red, green, blue, alpha;
			kinc_color_components(color, &red, &green, &blue, &alpha);
			renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
			renderPassDescriptor.colorAttachments[i].clearColor = MTLClearColorMake(red, green, blue, alpha);
		}
		else {
			renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
			renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
			renderPassDescriptor.colorAttachments[i].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
		}
	}

	if (clear_flags & KINC_G5_CLEAR_DEPTH) {
		renderPassDescriptor.depthAttachment.clearDepth = depth;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	}
	else {
		renderPassDescriptor.depthAttachment.clearDepth = 1;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	}
	renderPassDescriptor.depthAttachment.texture = renderTargets == NULL ? depthTexture : (__bridge id<MTLTexture>)renderTargets[0]->impl._depthTex;

	if (clear_flags & KINC_G5_CLEAR_STENCIL) {
		renderPassDescriptor.stencilAttachment.clearStencil = stencil;
		renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
		renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
	}
	else {
		renderPassDescriptor.stencilAttachment.clearStencil = 0;
		renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
		renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
	}
	renderPassDescriptor.stencilAttachment.texture = renderTargets == NULL ? depthTexture : (__bridge id<MTLTexture>)renderTargets[0]->impl._depthTex;

	id<MTLCommandQueue> commandQueue = getMetalQueue();
	commandBuffer = [commandQueue commandBuffer];
	commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}
