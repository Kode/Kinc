#include "pch.h"

#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>

#import <Metal/Metal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern kinc_g5_index_buffer_t *currentIndexBuffer;

id getMetalDevice();
id getMetalEncoder();
void newRenderPass(kinc_g5_render_target_t *renderTarget, bool wait);
void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline);

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

namespace {
	kinc_g5_render_target_t *lastRenderTarget = nullptr;
}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	lastRenderTarget = nullptr;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
								int stencil) {}

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
		indexBufferOffset:start];
}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	// TODO
	// id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	// MTLViewport viewport;
	// viewport.originX=x;
	// viewport.originY=y;
	// viewport.width=width;
	// viewport.height=height;
	// encoder.setViewport(viewport);
}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	kinc_g5_internal_pipeline_set(pipeline);
}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	kinc_g5_internal_vertex_buffer_set(buffers[0], offsets[0]);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	kinc_g5_internal_index_buffer_set(buffer);
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	if (targets[0]->contextId < 0) {
		lastRenderTarget = nullptr;
		newRenderPass(nullptr, false);
	}
	else {
		lastRenderTarget = targets[0];
		newRenderPass(targets[0], false);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	newRenderPass(lastRenderTarget, false);
}

void kinc_g5_command_list_execute_and_wait(kinc_g5_command_list_t *list) {
	newRenderPass(lastRenderTarget, true);
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
