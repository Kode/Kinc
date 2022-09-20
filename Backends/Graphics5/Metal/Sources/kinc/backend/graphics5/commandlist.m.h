#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/window.h>

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern kinc_g5_index_buffer_t *currentIndexBuffer;

id getMetalDevice(void);
id getMetalQueue(void);
id getMetalEncoder(void);

void kinc_g5_internal_new_render_pass(kinc_g5_render_target_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth,
                                      int stencil);
void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline);

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

static kinc_g5_render_target_t *lastRenderTargets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static kinc_g5_pipeline_t *lastPipeline = NULL;

static int formatSize(MTLPixelFormat format) {
	switch (format) {
	case MTLPixelFormatRGBA32Float:
		return 16;
	case MTLPixelFormatRGBA16Float:
		return 8;
	case MTLPixelFormatR16Float:
		return 2;
	case MTLPixelFormatR8Unorm:
		return 1;
	default:
		return 4;
	}
}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	lastRenderTargets[0] = NULL;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
                                int stencil) {
	if (renderTarget->framebuffer_index >= 0) {
		kinc_g5_internal_new_render_pass(NULL, 1, false, flags, color, depth, stencil);
	}
	else {
		kinc_g5_internal_new_render_pass(&renderTarget, 1, false, flags, color, depth, stencil);
	}
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, kinc_g5_index_buffer_count(currentIndexBuffer));
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {
	id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)currentIndexBuffer->impl.metal_buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
	                    indexCount:count
	                     indexType:(currentIndexBuffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32)
	                   indexBuffer:indexBuffer
	             indexBufferOffset:start * 4];
}

void kinc_g5_command_list_draw_indexed_vertices_from_to_from(kinc_g5_command_list_t *list, int start, int count, int vertex_offset) {
	id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)currentIndexBuffer->impl.metal_buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
	                    indexCount:count
	                     indexType:(currentIndexBuffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32)
	                   indexBuffer:indexBuffer
	             indexBufferOffset:start * 4
	                 instanceCount:1
	                    baseVertex:vertex_offset
	                  baseInstance:0];
}

void kinc_g5_command_list_draw_indexed_vertices_instanced(kinc_g5_command_list_t *list, int instanceCount) {
	kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(list, instanceCount, 0, kinc_g5_index_buffer_count(currentIndexBuffer));
}

void kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(kinc_g5_command_list_t *list, int instanceCount, int start, int count) {
	id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)currentIndexBuffer->impl.metal_buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
	                    indexCount:count
	                     indexType:(currentIndexBuffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32)
	                   indexBuffer:indexBuffer
	             indexBufferOffset:start * 4
	                 instanceCount:instanceCount
	                    baseVertex:0
	                  baseInstance:0];
}
void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLViewport viewport;
	viewport.originX = x;
	viewport.originY = y;
	viewport.width = width;
	viewport.height = height;
	[encoder setViewport:viewport];
}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLScissorRect scissor;
	scissor.x = x;
	scissor.y = y;
	scissor.width = width;
	scissor.height = height;
	[encoder setScissorRect:scissor];
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLScissorRect scissor;
	scissor.x = 0;
	scissor.y = 0;
	if (lastRenderTargets[0] != NULL) {
		scissor.width = lastRenderTargets[0]->texWidth;
		scissor.height = lastRenderTargets[0]->texHeight;
	}
	else {
		scissor.width = kinc_window_width(0);
		scissor.height = kinc_window_height(0);
	}
	[encoder setScissorRect:scissor];
}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	kinc_g5_internal_pipeline_set(pipeline);
	lastPipeline = pipeline;
}

void kinc_g5_command_list_set_blend_constant(kinc_g5_command_list_t *list, float r, float g, float b, float a) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setBlendColorRed:r green:g blue:b alpha:a];
}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	kinc_g5_internal_vertex_buffer_set(buffers[0], offsets[0]);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	kinc_g5_internal_index_buffer_set(buffer);
}

extern bool kinc_internal_metal_has_depth;

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	if (targets[0]->framebuffer_index >= 0) {
		for (int i = 0; i < 8; ++i) lastRenderTargets[i] = NULL;
		kinc_g5_internal_new_render_pass(NULL, 1, false, 0, 0, 0.0f, 0);
	}
	else {
		for (int i = 0; i < count; ++i) lastRenderTargets[i] = targets[i];
		for (int i = count; i < 8; ++i) lastRenderTargets[i] = NULL;
		kinc_g5_internal_new_render_pass(targets, count, false, 0, 0, 0.0f, 0);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	// Create readback buffer
	if (render_target->impl._texReadback == NULL) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = render_target->texWidth;
		descriptor.height = render_target->texHeight;
		descriptor.depth = 1;
		descriptor.pixelFormat = [(__bridge id<MTLTexture>)render_target->impl._tex pixelFormat];
		descriptor.arrayLength = 1;
		descriptor.mipmapLevelCount = 1;
		descriptor.usage = MTLTextureUsageUnknown;
#ifdef KINC_APPLE_SOC
		descriptor.resourceOptions = MTLResourceStorageModeShared;
#else
		descriptor.resourceOptions = MTLResourceStorageModeManaged;
#endif
		render_target->impl._texReadback = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];
	}

	// Copy render target to readback buffer
	id<MTLCommandQueue> commandQueue = getMetalQueue();
	id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
	id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
	[commandEncoder copyFromTexture:(__bridge id<MTLTexture>)render_target->impl._tex
	                    sourceSlice:0
	                    sourceLevel:0
	                   sourceOrigin:MTLOriginMake(0, 0, 0)
	                     sourceSize:MTLSizeMake(render_target->texWidth, render_target->texHeight, 1)
	                      toTexture:(__bridge id<MTLTexture>)render_target->impl._texReadback
	               destinationSlice:0
	               destinationLevel:0
	              destinationOrigin:MTLOriginMake(0, 0, 0)];
#ifndef KINC_APPLE_SOC
	[commandEncoder synchronizeResource:(__bridge id<MTLTexture>)render_target->impl._texReadback];
#endif
	[commandEncoder endEncoding];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];

	// Read buffer
	id<MTLTexture> tex = (__bridge id<MTLTexture>)render_target->impl._texReadback;
	int formatByteSize = formatSize([(__bridge id<MTLTexture>)render_target->impl._tex pixelFormat]);
	MTLRegion region = MTLRegionMake2D(0, 0, render_target->texWidth, render_target->texHeight);
	[tex getBytes:data bytesPerRow:formatByteSize * render_target->texWidth fromRegion:region mipmapLevel:0];
}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	if (lastRenderTargets[0] == NULL) {
		kinc_g5_internal_new_render_pass(NULL, 1, false, 0, 0, 0.0f, 0);
	}
	else {
		int count = 1;
		while (lastRenderTargets[count] != NULL) count++;
		kinc_g5_internal_new_render_pass(lastRenderTargets, count, false, 0, 0, 0.0f, 0);
	}
	if (lastPipeline != NULL) kinc_g5_internal_pipeline_set(lastPipeline);
}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:buf offset:offset atIndex:1];
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentBuffer:buf offset:offset atIndex:0];
}

void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {
#ifndef KINC_APPLE_SOC
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder textureBarrier];
#endif
}

void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	kinc_g5_internal_texture_set(texture, unit.impl.index);
}

void kinc_g5_command_list_set_image_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

bool kinc_g5_command_list_init_occlusion_query(kinc_g5_command_list_t *list, unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_command_list_delete_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery) {}

void kinc_g5_command_list_render_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery, int triangles) {}

bool kinc_g5_command_list_are_query_results_available(kinc_g5_command_list_t *list, unsigned occlusionQuery) {
	return false;
}

void kinc_g5_command_list_get_query_result(kinc_g5_command_list_t *list, unsigned occlusionQuery, unsigned *pixelCount) {}

extern bool kinc_internal_bilinear_filtering;

void kinc_g5_command_list_set_texture_minification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	kinc_internal_bilinear_filtering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

void kinc_g5_command_list_set_texture_mipmap_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

void kinc_g5_command_list_set_render_target_face(kinc_g5_command_list_t *list, kinc_g5_render_target_t *texture, int face) {}

void kinc_g5_command_list_set_texture_addressing(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir,
                                                 kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_command_list_set_texture_magnification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	kinc_internal_bilinear_filtering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}
