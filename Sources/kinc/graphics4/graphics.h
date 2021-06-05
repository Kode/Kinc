#pragma once

#include <kinc/global.h>

#include <kinc/math/matrix.h>

#include "constantlocation.h"
#include "textureunit.h"

/*! \file graphics.h
    \brief Contains the base G4-functionality.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_pipeline;
struct kinc_g4_render_target;
struct kinc_g4_texture;
struct kinc_g4_texture_array;

typedef enum {
	KINC_G4_TEXTURE_ADDRESSING_REPEAT,
	KINC_G4_TEXTURE_ADDRESSING_MIRROR,
	KINC_G4_TEXTURE_ADDRESSING_CLAMP,
	KINC_G4_TEXTURE_ADDRESSING_BORDER
} kinc_g4_texture_addressing_t;

typedef enum { KINC_G4_TEXTURE_DIRECTION_U, KINC_G4_TEXTURE_DIRECTION_V, KINC_G4_TEXTURE_DIRECTION_W } kinc_g4_texture_direction_t;

typedef enum {
	KINC_G4_TEXTURE_OPERATION_MODULATE,
	KINC_G4_TEXTURE_OPERATION_SELECT_FIRST,
	KINC_G4_TEXTURE_OPERATION_SELECT_SECOND
} kinc_g4_texture_operation_t;

typedef enum { KINC_G4_TEXTURE_ARGUMENT_CURRENT_COLOR, KINC_G4_TEXTURE_ARGUMENT_TEXTURE_COLOR } kinc_g4_texture_argument_t;

typedef enum { KINC_G4_TEXTURE_FILTER_POINT, KINC_G4_TEXTURE_FILTER_LINEAR, KINC_G4_TEXTURE_FILTER_ANISOTROPIC } kinc_g4_texture_filter_t;

typedef enum {
	KINC_G4_MIPMAP_FILTER_NONE,
	KINC_G4_MIPMAP_FILTER_POINT,
	KINC_G4_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} kinc_g4_mipmap_filter_t;

/// <summary>
/// Initializes the G4-API and is usually called at the start of a program.
/// </summary>
/// <param name="window">The window to initialize G4 for</param>
/// <param name="depthBufferBits">The number of bits for the depth buffer - 16 and 24 are typical values</param>
/// <param name="stencilBufferBits">The number of bits for the stencil-buffer - typically 8</param>
/// <param name="vSync">Whether or not to enable vertical-sync</param>
KINC_FUNC void kinc_g4_init(int window, int depthBufferBits, int stencilBufferBits, bool vSync);

/// <summary>
/// Destroy the G4-API for a window.
/// </summary>
/// <param name="window">The window to destroy the G4-API for</param>
/// <returns></returns>
KINC_FUNC void kinc_g4_destroy(int window);

/// <summary>
/// Kicks of lingering work - may or may not actually do anything depending on the underlying graphics-API.
/// </summary>
KINC_FUNC void kinc_g4_flush(void);

/// <summary>
/// Needs to be called before rendering to a window. Typically called at the start of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
KINC_FUNC void kinc_g4_begin(int window);

/// <summary>
/// Needs to be called after rendering to a window. Typically called at the end of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
/// <returns></returns>
KINC_FUNC void kinc_g4_end(int window);

/// <summary>
/// Needs to be called to make the rendered frame visible. Typically called at the very end of each frame.
/// </summary>
KINC_FUNC bool kinc_g4_swap_buffers(void);

#define KINC_G4_CLEAR_COLOR 1
#define KINC_G4_CLEAR_DEPTH 2
#define KINC_G4_CLEAR_STENCIL 4

/// <summary>
/// Clears the color, depth and/or stencil-components of the current framebuffer or render-target.
/// </summary>
/// <param name="flags">Defines what components to clear</param>
/// <param name="color">The color-value to clear to</param>
/// <param name="depth">The depth-value to clear to</param>
/// <param name="stencil">The stencil-value to clear to</param>
KINC_FUNC void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil);

/// <summary>
/// Sets the viewport which defines the portion of the framebuffer or render-target things are rendered into. By default the viewport is equivalent to the full
/// size of the current render-target or framebuffer.
/// </summary>
/// <param name="x">The x-offset of the viewport in pixels</param>
/// <param name="y">The y-offset of the viewport in pixels</param>
/// <param name="width">The width of the viewport in pixels</param>
/// <param name="height">The height of the viewport in pixels</param>
KINC_FUNC void kinc_g4_viewport(int x, int y, int width, int height);

/// <summary>
/// Enables and defines the scissor-rect. When the scissor-rect is enabled, anything that's rendered outside of the scissor-rect will be ignored.
/// </summary>
/// <param name="x">The x-offset of the scissor-rect in pixels</param>
/// <param name="y">The y-offset of the scissor-rect in pixels</param>
/// <param name="width">The width of the scissor-rect in pixels</param>
/// <param name="height">The height of the scissor-rect in pixels</param>
KINC_FUNC void kinc_g4_scissor(int x, int y, int width, int height);

/// <summary>
/// Disables the scissor-rect.
/// </summary>
/// <param name=""></param>
/// <returns></returns>
KINC_FUNC void kinc_g4_disable_scissor(void);

/// <summary>
/// Draws the entire content of the currently set index-buffer and vertex-buffer. G4 can only draw triangle-lists using vertex-indices as this is what GPUs tend
/// to be optimized for.
/// </summary>
KINC_FUNC void kinc_g4_draw_indexed_vertices(void);

KINC_FUNC void kinc_g4_draw_indexed_vertices_from_to(int start, int count);

KINC_FUNC void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_offset);

KINC_FUNC void kinc_g4_draw_indexed_vertices_instanced(int instanceCount);

KINC_FUNC void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count);

KINC_FUNC void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);

KINC_FUNC void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);

/// <summary>
/// Sets the pipeline for the next draw-call. The pipeline defines most rendering-state including the shaders to be used.
/// </summary>
/// <param name="pipeline">The pipeline to set</param>
KINC_FUNC void kinc_g4_set_pipeline(struct kinc_g4_pipeline *pipeline);

KINC_FUNC void kinc_g4_set_stencil_reference_value(int value);

KINC_FUNC void kinc_g4_set_texture_operation(kinc_g4_texture_operation_t operation, kinc_g4_texture_argument_t arg1, kinc_g4_texture_argument_t arg2);

KINC_FUNC void kinc_g4_set_int(kinc_g4_constant_location_t location, int value);
KINC_FUNC void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2);
KINC_FUNC void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3);
KINC_FUNC void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4);
KINC_FUNC void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count);

KINC_FUNC void kinc_g4_set_float(kinc_g4_constant_location_t location, float value);
KINC_FUNC void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2);
KINC_FUNC void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3);
KINC_FUNC void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4);
KINC_FUNC void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count);

KINC_FUNC void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value);

KINC_FUNC void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value);
KINC_FUNC void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value);

KINC_FUNC void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);

KINC_FUNC void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter);

KINC_FUNC void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);

KINC_FUNC void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter);

KINC_FUNC void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, kinc_g4_mipmap_filter_t filter);

KINC_FUNC void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter);

KINC_FUNC void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

KINC_FUNC void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

KINC_FUNC int kinc_g4_max_bound_textures(void);

KINC_FUNC bool kinc_g4_render_targets_inverted_y(void);

KINC_FUNC bool kinc_g4_non_pow2_textures_supported(void);

/// <summary>
/// Sets the framebuffer (aka the actual contents of the current window) to be the target of any future draw-calls.
/// </summary>
KINC_FUNC void kinc_g4_restore_render_target(void);

/// <summary>
/// Sets the passed render-targets to be the target of any future draw-calls.
/// </summary>
/// <param name="targets">An array of render-targets</param>
/// <param name="count">The number of render-targets in the render-target-array</param>
KINC_FUNC void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count);

KINC_FUNC void kinc_g4_set_render_target_face(struct kinc_g4_render_target *texture, int face);

KINC_FUNC void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

KINC_FUNC void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

KINC_FUNC bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery);

KINC_FUNC void kinc_g4_delete_occlusion_query(unsigned occlusionQuery);

KINC_FUNC void kinc_g4_start_occlusion_query(unsigned occlusionQuery);

KINC_FUNC void kinc_g4_end_occlusion_query(unsigned occlusionQuery);

KINC_FUNC bool kinc_g4_are_query_results_available(unsigned occlusionQuery);

KINC_FUNC void kinc_g4_get_query_results(unsigned occlusionQuery, unsigned *pixelCount);

KINC_FUNC void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, struct kinc_g4_texture_array *array);

KINC_FUNC int kinc_g4_antialiasing_samples(void);

KINC_FUNC void kinc_g4_set_antialiasing_samples(int samples);

#ifdef __cplusplus
}
#endif
