#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>

#include <kinc/graphics4/graphics.h>

#include <assert.h>
#include <string.h>

#ifdef KORE_MICROSOFT
#include <malloc.h>
#endif

enum Commands { Clear, Draw, SetViewport, SetScissor, SetPipeline, SetVertexBuffer, SetIndexBuffer, SetRenderTarget, DrawInstanced };

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
			kinc_g4_clear((unsigned)list->impl.commands[index + 1], (unsigned)list->impl.commands[index + 2], 0.0f, 0);
			index += 3;
			break;
		case Draw:
			kinc_g4_draw_indexed_vertices_from_to((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2]);
			index += 3;
			break;
		case SetViewport:
			kinc_g4_viewport((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2], (int)list->impl.commands[index + 3],
			                 (int)list->impl.commands[index + 4]);
			index += 5;
			break;
		case SetScissor:
			kinc_g4_scissor((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2], (int)list->impl.commands[index + 3],
			                (int)list->impl.commands[index + 4]);
			index += 5;
			break;
		case SetPipeline: {
			kinc_g5_pipeline_t *pipeline = (kinc_g5_pipeline_t *)list->impl.commands[index + 1];
			kinc_g4_set_pipeline(&pipeline->impl.pipe);
			index += 2;
			break;
		}
		case SetVertexBuffer: {
			// kinc_g5_vertex_buffer_t *vb = (kinc_g5_vertex_buffer_t *)list->impl.commands[index + 1];
			int count = (int)list->impl.commands[index + 1];
#ifdef KORE_MICROSOFT
			kinc_g4_vertex_buffer_t **buffers = (kinc_g4_vertex_buffer_t **)alloca(sizeof(kinc_g4_vertex_buffer_t *) * count);
#else
			kinc_g4_vertex_buffer_t *buffers[count];
#endif
			for (int i = 0; i < count; ++i) {
				buffers[i] = &(((kinc_g5_vertex_buffer_t *)list->impl.commands[index + 1 + i])->impl.buffer);
			}
			kinc_g4_set_vertex_buffers(buffers, count);
			index += (2 + count);
			break;
		}
		case SetIndexBuffer: {
			kinc_g5_index_buffer_t *ib = (kinc_g5_index_buffer_t *)list->impl.commands[index + 1];
			kinc_g4_set_index_buffer(&ib->impl.buffer);
			index += 2;
			break;
		}
		case SetRenderTarget:

			index += 2;
			break;
		case DrawInstanced:
			kinc_g4_draw_indexed_vertices_instanced_from_to((int)list->impl.commands[index + 1], (int)list->impl.commands[index + 2],
			                                                (int)list->impl.commands[index + 3]);
			index += 4;
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

void kinc_g5_command_list_draw_indexed_vertices_instanced(kinc_g5_command_list_t *list, int instanceCount) {
	list->impl.commands[list->impl.commandIndex++] = DrawInstanced;
	list->impl.commands[list->impl.commandIndex++] = instanceCount;
	list->impl.commands[list->impl.commandIndex++] = 0;
	list->impl.commands[list->impl.commandIndex++] = list->impl._indexCount;
}
void kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(kinc_g5_command_list_t *list, int instanceCount, int start, int count) {
	list->impl.commands[list->impl.commandIndex++] = DrawInstanced;
	list->impl.commands[list->impl.commandIndex++] = instanceCount;
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
	list->impl.commands[list->impl.commandIndex++] = (int64_t)pipeline;
}

void kinc_g5_command_list_set_blend_constant(kinc_g5_command_list_t *list, float r, float g, float b, float a) {}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	list->impl.commands[list->impl.commandIndex++] = SetVertexBuffer;
	list->impl.commands[list->impl.commandIndex++] = count;
	for (int i = 0; i < count; ++i) {
		list->impl.commands[list->impl.commandIndex++] = (int64_t)buffers[i];
		// list->impl.commands[list->impl.commandIndex++] = (int64_t)offsets[i];
	}
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	list->impl.commands[list->impl.commandIndex++] = SetIndexBuffer;
	list->impl.commands[list->impl.commandIndex++] = (int64_t)&buffer;
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	list->impl.commands[list->impl.commandIndex++] = SetRenderTarget;
	list->impl.commands[list->impl.commandIndex++] = (int64_t)targets[0];
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}
void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_render_target_face(kinc_g5_command_list_t *list, kinc_g5_render_target_t *texture, int face) {}

/*
void Graphics5::setVertexBuffers(VertexBuffer** buffers, int count) {
    buffers[0]->_set(0);
}

void Graphics5::setIndexBuffer(IndexBuffer& buffer) {
    buffer._set();
}
*/

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g4_texture_unit_t g4_unit;
	memcpy(&g4_unit.stages[0], &unit.stages[0], KINC_G4_SHADER_TYPE_COUNT * sizeof(int));
	kinc_g4_set_texture(g4_unit, &texture->impl.texture);
}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {}

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *render_target) {
	// kinc_g4_render_target_use_color_as_texture(render_target->impl, unit.impl.unit);
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit,
                                                               kinc_g5_render_target_t *render_target) {
	// kinc_g4_render_target_use_depth_as_texture(render_target->impl, unit.impl.unit);
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

/*void Graphics5::setPipeline(PipelineState* pipeline) {
    pipeline->set(pipeline);
}*/
