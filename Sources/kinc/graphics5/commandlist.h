#pragma once

#include <kinc/global.h>

#include "rendertarget.h"
#include "texture.h"
#include "textureunit.h"

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

typedef enum kinc_g5_texture_addressing {
	KINC_G5_TEXTURE_ADDRESSING_REPEAT,
	KINC_G5_TEXTURE_ADDRESSING_MIRROR,
	KINC_G5_TEXTURE_ADDRESSING_CLAMP,
	KINC_G5_TEXTURE_ADDRESSING_BORDER
} kinc_g5_texture_addressing_t;

typedef enum kinc_g5_texture_filter {
	KINC_G5_TEXTURE_FILTER_POINT,
	KINC_G5_TEXTURE_FILTER_LINEAR,
	KINC_G5_TEXTURE_FILTER_ANISOTROPIC
} kinc_g5_texture_filter_t;

typedef enum kinc_g5_mipmap_filter {
	KINC_G5_MIPMAP_FILTER_NONE,
	KINC_G5_MIPMAP_FILTER_POINT,
	KINC_G5_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} kinc_g5_mipmap_filter_t;

typedef enum kinc_g5_texture_direction { KINC_G5_TEXTURE_DIRECTION_U, KINC_G5_TEXTURE_DIRECTION_V, KINC_G5_TEXTURE_DIRECTION_W } kinc_g5_texture_direction_t;

/*typedef enum kinc_g5_render_target_format {
    KINC_G5_RENDER_TARGET_FORMAT_32BIT,
    KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT,
    KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
    KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT,
    KINC_G5_RENDER_TARGET_FORMAT_16BIT_DEPTH,
    KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED
} kinc_g5_render_target_format_t;*/
// typedef kinc_g4_render_target_format_t kinc_g5_render_target_format_t;

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
/// Ends recording commands for the list. Has to be called after kinc_g5_command_list_begin and before kinc_g5_command_list_execute.
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

/// <summary>
/// Records a command that prepares a render-target to be used as the current framebuffer.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="renderTarget">The render-target to use as the current framebuffer</param>
KINC_FUNC void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

/// <summary>
/// Records a command that prepares a render-target for regular render-target-usage after being used as the current framebuffer.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="renderTarget">The render-target to use in regular render-target-mode</param>
KINC_FUNC void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

/// <summary>
/// Writes a command that prepares a render-target to be rendered to.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="renderTarget">The render-target to render to</param>
KINC_FUNC void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

/// <summary>
/// Writes a command that prepares a render-target to be used for sampling/reading like a texture.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="renderTarget">The render-target to be used like a texture</param>
KINC_FUNC void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget);

/// <summary>
/// Writes a command that draws the entire content of the currently set index-buffer and vertex-buffer. G4 can only draw triangle-lists using vertex-indices as
/// this is what GPUs tend to be optimized for.
/// </summary>
/// <param name="list">The list to write the command to</param>
KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list);

/// <summary>
/// Writes a command that draws a part of the content of the currently set index-buffer and vertex-buffer. G4 can only draw triangle-lists using vertex-indices
/// as this is what GPUs tend to be optimized for.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="start">The offset into the index-buffer</param>
/// <param name="count">The number of indices to use</param>
KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count);

/// <summary>
/// Writes a command that draws a part of the content of the currently set index-buffer and vertex-buffer and additionally applies a general offset into the
/// vertex-buffer. G4 can only draw triangle-lists using vertex-indices as this is what GPUs tend to be optimized for.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="start">The offset into the index-buffer</param>
/// <param name="count">The number of indices to use</param>
/// <param name="vertex_offset">The offset into the vertex-buffer which is added to each index read from the index-buffer</param>
KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_from_to_from(kinc_g5_command_list_t *list, int start, int count, int vertex_offset);

KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_instanced(kinc_g5_command_list_t *list, int instanceCount);
KINC_FUNC void kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(kinc_g5_command_list_t *list, int instanceCount, int start, int count);

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

/// <summary>
/// Sets the blend constant used for `KINC_G5_BLEND_CONSTANT` or `KINC_G5_INV_BLEND_CONSTANT`
/// </summary>
KINC_FUNC void kinc_g5_command_list_set_blend_constant(kinc_g5_command_list_t *list, float r, float g, float b, float a);

/// <summary>
/// Writes a command which sets vertex-buffers for the next draw-call.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffers">The buffers to set</param>
/// <param name="offsets">The offset to use for every buffer in number of vertices</param>
/// <param name="count">The number of buffers to set</param>
KINC_FUNC void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count);

/// <summary>
/// Writes a command to set an index-buffer to be used for the next draw-command.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The buffer to use</param>
KINC_FUNC void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);

/// <summary>
/// Writes a command that sets the render-targets to draw into in following draw-calls.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="targets">The render-targets to use for following-draw calls</param>
/// <param name="count">The number of render-targets to use</param>
KINC_FUNC void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count);

/// <summary>
/// Writes a command to upload an index-buffer that's in main-memory to gpu-memory. Does nothing on unified-memory-systems.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The buffer to upload</param>
KINC_FUNC void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);

/// <summary>
/// Writes a command to upload a vertex-buffer that's in main-memory to gpu-memory. Does nothing on unified-memory-systems.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The buffer to upload</param>
KINC_FUNC void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer);

/// <summary>
/// Writes a command to upload a texture that's in main-memory to gpu-memory. Does nothing on unified-memory-systems.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The texture to upload</param>
KINC_FUNC void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture);

/// <summary>
/// Writes a command that sets a constant-buffer for the vertex-shader-stage.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The buffer to set</param>
/// <param name="offset">The offset into the buffer in bytes to use as the start</param>
/// <param name="size">The size of the buffer to use in bytes starting at the offset</param>
KINC_FUNC void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);

/// <summary>
/// Writes a command that sets a constant-buffer for the fragment-shader-stage.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="buffer">The buffer to set</param>
/// <param name="offset">The offset into the buffer in bytes to use as the start</param>
/// <param name="size">The size of the buffer to use in bytes starting at the offset</param>
KINC_FUNC void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);

/// <summary>
/// This currently has to be called after setting a pipeline but is target to be removed by the main-developer.
/// </summary>
/// <param name="list">The list to write the command to</param>
KINC_FUNC void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list);

/// <summary>
/// Kicks of execution of the commands which have been recorded in the command-list. kinc_g5_command_list_end has to be called beforehand.
/// </summary>
/// <param name="list">The command-list to execute</param>
KINC_FUNC void kinc_g5_command_list_execute(kinc_g5_command_list_t *list);

/// <summary>
/// Waits for execution of the command_list to finish. Make sure the command-list is executing before you wait for it.
/// Also take note that waiting for a command-list to finish executing completely is a very expensive operation.
/// </summary>
/// <param name="list">The command-list to execute</param>
KINC_FUNC void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list);

/// <summary>
/// Writes a command that copies the contents of a render-target into a cpu-side buffer. Beware: This is enormously slow.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="render_target">The render-target to copy the data from</param>
/// <param name="data">The buffer to copy the data into</param>
KINC_FUNC void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, struct kinc_g5_render_target *render_target, uint8_t *data);

/// <summary>
/// Records a command that fires off a compute-run on x * y * z elements.
/// </summary>
/// <param name="list">The list to write the command to</param>
/// <param name="x">The x-size for the compute-run</param>
/// <param name="y">The y-size for the compute-run</param>
/// <param name="z">The z-size for the compute-run</param>
KINC_FUNC void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z);

/// <summary>
/// Assigns a texture to a texture-unit for sampled access.
/// </summary>
/// <param name="unit">The unit to assign this texture to</param>
/// <param name="texture">The texture to assign to the unit</param>
KINC_FUNC void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture);

/// <summary>
/// Assigns a texture to a texture-unit for direct access.
/// </summary>
/// <param name="unit">The unit to assign this texture to</param>
/// <param name="texture">The texture to assign to the unit</param>
KINC_FUNC void kinc_g5_command_list_set_image_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture);

KINC_FUNC void kinc_g5_command_list_set_render_target_face(kinc_g5_command_list_t *list, kinc_g5_render_target_t *texture, int face);

KINC_FUNC void kinc_g5_command_list_set_texture_addressing(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir,
                                                           kinc_g5_texture_addressing_t addressing);

/// <summary>
/// Set the texture-sampling-mode for upscaled textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
KINC_FUNC void kinc_g5_command_list_set_texture_magnification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit,
                                                                     kinc_g5_texture_filter_t filter);

/// <summary>
/// Set the texture-sampling-mode for downscaled textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
KINC_FUNC void kinc_g5_command_list_set_texture_minification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit,
                                                                    kinc_g5_texture_filter_t filter);

/// <summary>
/// Sets the mipmap-sampling-mode which defines whether mipmaps are used at all and if so whether the two neighbouring mipmaps are linearly interoplated.
/// </summary>
/// <param name="unit">The texture-unit to set the mipmap-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
KINC_FUNC void kinc_g5_command_list_set_texture_mipmap_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter);

// Occlusion Query
KINC_FUNC bool kinc_g5_command_list_init_occlusion_query(kinc_g5_command_list_t *list, unsigned *occlusionQuery);
KINC_FUNC void kinc_g5_command_list_delete_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery);
KINC_FUNC void kinc_g5_command_list_render_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery, int triangles);
KINC_FUNC bool kinc_g5_command_list_are_query_results_available(kinc_g5_command_list_t *list, unsigned occlusionQuery);
KINC_FUNC void kinc_g5_command_list_get_query_result(kinc_g5_command_list_t *list, unsigned occlusionQuery, unsigned *pixelCount);

#ifdef __cplusplus
}
#endif
