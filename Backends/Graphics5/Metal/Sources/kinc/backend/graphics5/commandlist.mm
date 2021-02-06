#include "pch.h"

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

id getMetalDevice();
id getMetalQueue();
id getMetalEncoder();
void kinc_g5_internal_new_render_pass(kinc_g5_render_target_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth,
                                      int stencil);
void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline);

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

namespace {
	kinc_g5_render_target_t *lastRenderTargets[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	kinc_g5_pipeline_t *lastPipeline = nullptr;

	int formatSize(MTLPixelFormat format) {
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
}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	lastRenderTargets[0] = nullptr;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
								int stencil) {
    if (renderTarget->contextId < 0) {
        kinc_g5_internal_new_render_pass(nullptr, 1, false, flags, color, depth, stencil);
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
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
		indexCount:count indexType:MTLIndexTypeUInt32
		indexBuffer:currentIndexBuffer->impl.mtlBuffer
		indexBufferOffset:start * 4];
}

void kinc_g5_command_list_draw_indexed_vertices_from_to_from(kinc_g5_command_list_t *list, int start, int count, int vertex_offset) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
		indexCount:count indexType:MTLIndexTypeUInt32
		indexBuffer:currentIndexBuffer->impl.mtlBuffer
		indexBufferOffset:start * 4 instanceCount: 1 baseVertex: vertex_offset baseInstance: 0];
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
	if (lastRenderTargets[0] != nullptr) {
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

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	kinc_g5_internal_vertex_buffer_set(buffers[0], offsets[0]);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	kinc_g5_internal_index_buffer_set(buffer);
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	if (targets[0]->contextId < 0) {
		for (int i = 0; i < 8; ++i) lastRenderTargets[i] = nullptr;
		kinc_g5_internal_new_render_pass(nullptr, 1, false, 0, 0, 0.0f, 0);
	}
	else {
		for (int i = 0; i < count; ++i) lastRenderTargets[i] = targets[i];
		for (int i = count; i < 8; ++i) lastRenderTargets[i] = nullptr;
		kinc_g5_internal_new_render_pass(targets, count, false, 0, 0, 0.0f, 0);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	// Create readback buffer
	if (render_target->impl._texReadback == nullptr) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = render_target->texWidth;
		descriptor.height = render_target->texHeight;
		descriptor.depth = 1;
		descriptor.pixelFormat = [(id<MTLTexture>)render_target->impl._tex pixelFormat];
		descriptor.arrayLength = 1;
		descriptor.mipmapLevelCount = 1;
		descriptor.usage = MTLTextureUsageUnknown;
#ifdef KORE_IOS
		descriptor.resourceOptions = MTLResourceStorageModeShared;
#else
		descriptor.resourceOptions = MTLResourceStorageModeManaged;
#endif
		render_target->impl._texReadback = [device newTextureWithDescriptor:descriptor];
	}

	// Copy render target to readback buffer
	id<MTLCommandQueue> commandQueue = getMetalQueue();
	id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
	id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
	[commandEncoder copyFromTexture:render_target->impl._tex sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0) sourceSize:MTLSizeMake(render_target->texWidth, render_target->texHeight, 1) toTexture:render_target->impl._texReadback destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(0, 0, 0)];
#ifndef KORE_IOS
	[commandEncoder synchronizeResource:render_target->impl._texReadback];
#endif
	[commandEncoder endEncoding];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];

	// Read buffer
	id<MTLTexture> tex = render_target->impl._texReadback;
	int formatByteSize = formatSize([(id<MTLTexture>)render_target->impl._tex pixelFormat]);
	MTLRegion region = MTLRegionMake2D(0, 0, render_target->texWidth, render_target->texHeight);
	[tex getBytes:data bytesPerRow:formatByteSize * render_target->texWidth fromRegion:region mipmapLevel:0];
}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	if (lastRenderTargets[0] == nullptr) {
		kinc_g5_internal_new_render_pass(nullptr, 1, false, 0, 0, 0.0f, 0);
	}
	else {
		int count = 1;
		while (lastRenderTargets[count] != nullptr) count++;
		kinc_g5_internal_new_render_pass(lastRenderTargets, count, false, 0, 0, 0.0f, 0);
	}
	if (lastPipeline != nullptr) kinc_g5_internal_pipeline_set(lastPipeline);
}

void kinc_g5_command_list_execute_and_wait(kinc_g5_command_list_t *list) {
	if (lastRenderTargets[0] == nullptr) {
		kinc_g5_internal_new_render_pass(nullptr, 1, true, 0, 0, 0.0f, 0);
	}
	else {
		int count = 1;
		while (lastRenderTargets[count] != nullptr) count++;
		kinc_g5_internal_new_render_pass(lastRenderTargets, count, true, 0, 0, 0.0f, 0);
	}
	if (lastPipeline != nullptr) kinc_g5_internal_pipeline_set(lastPipeline);
}

void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:buffer->impl._buffer offset:offset atIndex:1];
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentBuffer:buffer->impl._buffer offset:offset atIndex:0];
}

void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {
#ifndef KORE_IOS
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder textureBarrier];
#endif
}

void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
