#pragma once

#include <kinc/global.h>

#include "buffer.h"
#include "renderpipeline.h"
#include "texture.h"

#include <kinc/backend/graphics6/commandbuffer.h>

/*! \file commandbuffer.h
    \brief Provides functions for setting up and using command buffers.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g6_command_buffer {
	kinc_g6_command_buffer_impl_t impl;
} kinc_g6_command_buffer_t;

// Initialize the command buffer
KINC_FUNC void kinc_g6_command_buffer_init(kinc_g6_command_buffer_t *buffer);
// Begin recording commands
KINC_FUNC void kinc_g6_command_buffer_begin(kinc_g6_command_buffer_t *buffer);
// Stop recording commands
KINC_FUNC void kinc_g6_command_buffer_end(kinc_g6_command_buffer_t *buffer);

typedef enum kinc_g6_load_op { KINC_G6_LOAD_OP_LOAD, KINC_G6_LOAD_OP_CLEAR, KINC_G6_LOAD_OP_DONT_CARE } kinc_g6_load_op_t;

typedef enum kinc_g6_store_op { KINC_G6_STORE_OP_STORE, KINC_G6_STORE_OP_CLEAR, KINC_G6_STORE_OP_DONT_CARE } kinc_g6_store_op_t;

typedef struct kinc_g6_color_attachment {
	kinc_g6_texture_view_t *texture;

	kinc_g6_load_op_t load_op;
	kinc_g6_store_op_t store_op;

	uint32_t clear_value;
} kinc_g6_color_attachment_t;

typedef struct kinc_g6_depth_stencil_attachment {
	kinc_g6_texture_t *texture;

	kinc_g6_load_op_t depth_load_op;
	kinc_g6_store_op_t depth_store_op;

	kinc_g6_load_op_t stencil_load_op;
	kinc_g6_store_op_t stencil_store_op;

	float depth_clear_value;
	uint32_t stencil_clear_value;
} kinc_g6_depth_stencil_attachment_t;

typedef struct kinc_g6_render_pass_descriptor {
	int color_attachment_count;
	kinc_g6_color_attachment_t color_attachments[8];

	kinc_g6_depth_stencil_attachment_t depth_stencil_attachment;
} kinc_g6_render_pass_descriptor_t;

KINC_FUNC void kinc_g6_command_buffer_render_pass_begin(kinc_g6_command_buffer_t *buffer, const kinc_g6_render_pass_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_command_buffer_render_pass_end(kinc_g6_command_buffer_t *buffer);

KINC_FUNC void kinc_g6_command_buffer_set_render_pipeline(kinc_g6_command_buffer_t *buffer, kinc_g6_render_pipeline_t *pipeline);
KINC_FUNC void kinc_g6_command_buffer_set_index_buffer(kinc_g6_command_buffer_t *buffer, kinc_g6_buffer_t *index_buffer, int offset);
KINC_FUNC void kinc_g6_command_buffer_set_vertex_buffers(kinc_g6_command_buffer_t *buffer, kinc_g6_buffer_t **vertex_buffers, int **offsets, int count);

struct kinc_g6_bind_group;

KINC_FUNC void kinc_g6_command_buffer_set_bind_group(kinc_g6_command_buffer_t *buffer, int index, struct kinc_g6_bind_group *group, uint32_t dynamicOffsetsCount, uint32_t* dynamicOffsets);

KINC_FUNC void kinc_g6_command_buffer_set_viewport(kinc_g6_command_buffer_t *buffer, int x, int y, int width, int height, int min_depth, int max_depth);
KINC_FUNC void kinc_g6_command_buffer_set_scissor(kinc_g6_command_buffer_t *buffer, int x, int y, int width, int height);

// KINC_FUNC void kinc_g6_command_buffer_set_blend_constant(kinc_g6_command_buffer_t* buffer, uint32_t constant);
// KINC_FUNC void kinc_g6_command_buffer_set_stencil_reference(kinc_g6_command_buffer_t *buffer,)

KINC_FUNC void kinc_g6_command_buffer_draw_indexed_vertices(kinc_g6_command_buffer_t *buffer, int start, int count, int vertex_offset);
KINC_FUNC void kinc_g6_command_buffer_draw_indexed_vertices_instanced(kinc_g6_command_buffer_t *list, int instanceCount, int start, int count);

#ifdef __cplusplus
}
#endif