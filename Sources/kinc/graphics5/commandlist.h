#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/commandlist.h>

#include <stddef.h>

/*! \file commandlist.h
    \brief Contains functions for building command-lists to send commands to the GPU.
*/

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

/// <summary>
/// Initializes a command-list.
/// </summary>
/// <param name="list">The command-list to initialize</param>
KINC_FUNC void kinc_g5_command_list_init(kinc_g5_command_list_t *list);

/// <summary>
/// Destroys a command-list.
/// </summary>
/// <param name="list">The command-list to destroy</param>
KINC_FUNC void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list);

/// <summary>
/// Starts recording commands in a command-list.
/// </summary>
/// <param name="list">The list to use</param>
KINC_FUNC void kinc_g5_command_list_begin(kinc_g5_command_list_t *list);

/// <summary>
/// Ends recording commands for the list. Has to be called after kinc_g5_command_list_begin and before kinc_g5_command_list_execute or
/// kinc_g5_command_list_execute_and_wait.
/// </summary>
/// <param name="list"></param>
/// <returns></returns>
KINC_FUNC void kinc_g5_command_list_end(kinc_g5_command_list_t *list);

/// <summary>
/// Records a command to clear the color, depth and/or stencil-components of a render-target.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="render_target">The render-target to clear</param>
/// <param name="flags">Defines what components to clear</param>
/// <param name="color">The color-value to clear to</param>
/// <param name="depth">The depth-value to clear to</param>
/// <param name="stencil">The stencil-value to clear to</param>
KINC_FUNC void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *render_target, unsigned flags, unsigned color,
                                          float depth, int stencil);

KINC_FUNC void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

KINC_FUNC void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

KINC_FUNC void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

KINC_FUNC void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list);

KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count);

KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_from_to_from(kinc_g5_command_list_t *list, int start, int count, int vertex_offset);

/// <summary>
/// Writes a command that sets the viewport which defines the portion of the framebuffer or render-target things are rendered into. By default the viewport is
/// equivalent to the full size of the current render-target or framebuffer.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="x">The x-offset of the viewport in pixels</param>
/// <param name="y">The y-offset of the viewport in pixels</param>
/// <param name="width">The width of the viewport in pixels</param>
/// <param name="height">The height of the viewport in pixels</param>
KINC_FUNC void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height);

/// <summary>
/// Writes a command that enables and defines the scissor-rect. When the scissor-rect is enabled, anything that's rendered outside of the scissor-rect will be
/// ignored.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="x">The x-offset of the scissor-rect in pixels</param>
/// <param name="y">The y-offset of the scissor-rect in pixels</param>
/// <param name="width">The width of the scissor-rect in pixels</param>
/// <param name="height">The height of the scissor-rect in pixels</param>
KINC_FUNC void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height);

/// <summary>
/// Writes a command to disable the scissor-rect.
/// </summary>
/// <param name="list">The list to write the command to</param>
KINC_FUNC void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list);

/// <summary>
/// Writes a command to set the pipeline for the next draw-call. The pipeline defines most rendering-state including the shaders to be used.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="pipeline">The pipeline to set</param>
KINC_FUNC void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline);

KINC_FUNC void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count);

KINC_FUNC void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);

// void restoreRenderTarget();

KINC_FUNC void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count);

KINC_FUNC void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);

KINC_FUNC void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer);

KINC_FUNC void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture);

KINC_FUNC void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);

KINC_FUNC void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);

KINC_FUNC void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list);

/// <summary>
/// Kicks of execution of the commands which have been recorded in the command-list. kinc_g5_command_list_end has to be called beforehand.
/// </summary>
/// <param name="list">The command-list to execute</param>
KINC_FUNC void kinc_g5_command_list_execute(kinc_g5_command_list_t *list);

/// <summary>
/// Kicks of execution of the commands which have been recorded in the command-list and waits for it to finish. kinc_g5_command_list_end has to be called
/// beforehand.
/// </summary>
/// <param name="list">The command-list to execute</param>
KINC_FUNC void kinc_g5_command_list_execute_and_wait(kinc_g5_command_list_t *list);

KINC_FUNC void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, struct kinc_g5_render_target *render_target, uint8_t *data);

/// <summary>
/// Records a command that fires off a compute-run on x * y * z elements.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="x">The x-size for the compute-run</param>
/// <param name="y">The y-size for the compute-run</param>
/// <param name="z">The z-size for the compute-run</param>
KINC_FUNC void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z);

#ifdef __cplusplus
}
#endif
