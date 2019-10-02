#pragma once

#include "pch.h"

#include <Kore/CommandList5Impl.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_G5_CLEAR_COLOR 1
#define KINC_G5_CLEAR_DEPTH 2
#define KINC_G5_CLEAR_STENCIL 4

struct kinc_g5_constant_buffer;
struct kinc_g5_index_buffer;
struct kinc_g5_pipeline;
struct kinc_g5_render_target;
struct kinc_g5_texture;
struct kinc_g5_vertex_buffer;
struct kinc_g5_render_target;

typedef struct kinc_g5_command_list {
	CommandList5Impl impl;
} kinc_g5_command_list_t;

void kinc_g5_command_list_init(kinc_g5_command_list_t *list);
void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list);
void kinc_g5_command_list_begin(kinc_g5_command_list_t *list);
void kinc_g5_command_list_end(kinc_g5_command_list_t *list);
void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
                                int stencil);
void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);
void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list);
void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count);
void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height);
void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height);
void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list);
void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline);
void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count);
void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);
// void restoreRenderTarget();
void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count);
void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer);
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture);
void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);
void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);
void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list);
void kinc_g5_command_list_execute(kinc_g5_command_list_t *list);
void kinc_g5_command_list_execute_and_wait(kinc_g5_command_list_t *list);
void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, struct kinc_g5_render_target *render_target, uint8_t *data);

#ifdef __cplusplus
}
#endif
