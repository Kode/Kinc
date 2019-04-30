#include "pch.h"

#include <Kinc/Graphics5/CommandList.h>
#include <Kinc/Graphics5/Pipeline.h>

#include <Kore/Graphics4/Graphics.h>

namespace {
	enum Commands { Clear, Draw, SetViewport, SetScissor, SetPipeline, SetVertexBuffer, SetIndexBuffer, SetRenderTarget };
}

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	list->impl.commandIndex = 0;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {
	int index = 0;
	while (index < list->impl.commandIndex) {
		switch (list->impl.commands[index]) {
		case Clear:
			Graphics4::clear((unsigned)list->impl.commands[index + 1], (unsigned)list->impl.commands[index + 2]);
			index += 3;
			break;
		case Draw:
			Graphics4::drawIndexedVertices((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2]);
			index += 3;
			break;
		case SetViewport:
			Graphics4::viewport((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2], (int)list->impl.commands[index + 3],
			                    (int)list->impl.commands[index + 4]);
			index += 5;
			break;
		case SetScissor:
			Graphics4::scissor((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2], (int)list->impl.commands[index + 3],
			                   (int)list->impl.commands[index + 4]);
			index += 5;
			break;
		case SetPipeline: {
			PipelineState *pipeline = (PipelineState *)list->impl.commands[index + 1];
			Graphics4::setPipeline(pipeline->state);
			index += 2;
			break;
		}
		case SetVertexBuffer: {
			VertexBuffer *vb = (VertexBuffer *)list->impl.commands[index + 1];
			Graphics4::setVertexBuffer(*vb->buffer);
			index += 2;
			break;
		}
		case SetIndexBuffer: {
			IndexBuffer *ib = (IndexBuffer *)list->impl.commands[index + 1];
			Graphics4::setIndexBuffer(*ib->buffer);
			index += 2;
			break;
		}
		case SetRenderTarget:

			index += 2;
			break;
		default:
			return;
		}
	}
}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
                                int stencil) {
	list->impl.commands[list->impl.commandIndex++] = Clear;
	list->impl.commands[list->impl.commandIndex++] = flags;
	list->impl.commands[list->impl.commandIndex++] = color;
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	list->impl.commands[list->impl.commandIndex++] = Draw;
	list->impl.commands[list->impl.commandIndex++] = 0;
	list->impl.commands[list->impl.commandIndex++] = list->impl._indexCount;
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {
	list->impl.commands[list->impl.commandIndex++] = Draw;
	list->impl.commands[list->impl.commandIndex++] = start;
	list->impl.commands[list->impl.commandIndex++] = count;
}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	list->impl.commands[list->impl.commandIndex++] = SetViewport;
	list->impl.commands[list->impl.commandIndex++] = x;
	list->impl.commands[list->impl.commandIndex++] = y;
	list->impl.commands[list->impl.commandIndex++] = width;
	list->impl.commands[list->impl.commandIndex++] = height;
}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	list->impl.commands[list->impl.commandIndex++] = SetScissor;
	list->impl.commands[list->impl.commandIndex++] = x;
	list->impl.commands[list->impl.commandIndex++] = y;
	list->impl.commands[list->impl.commandIndex++] = width;
	list->impl.commands[list->impl.commandIndex++] = height;
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	list->impl.commands[list->impl.commandIndex++] = SetPipeline;
	list->impl.commands[list->impl.commandIndex++] = (s64)pipeline;
}

void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	list->impl.commands[list->impl.commandIndex++] = SetVertexBuffer;
	list->impl.commands[list->impl.commandIndex++] = (s64)buffers[0];
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	list->impl.commands[list->impl.commandIndex++] = SetIndexBuffer;
	list->impl.commands[list->impl.commandIndex++] = (s64)&buffer;
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	list->impl.commands[list->impl.commandIndex++] = SetRenderTarget;
	list->impl.commands[list->impl.commandIndex++] = (s64)targets[0];
}

void kinc_g5_command_list_upload(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}
void kinc_g5_command_list_upload(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}
void kinc_g5_command_list_upload(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}
