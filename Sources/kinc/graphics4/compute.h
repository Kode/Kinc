#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/compute.h>
#ifdef KORE_OPENGL
#include <kinc/backend/graphics4/ShaderStorageBufferImpl.h>
#endif
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/vertexstructure.h>
#include <kinc/math/matrix.h>

/*! \file compute.h
    \brief Provides support for running compute-shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_texture;
struct kinc_g4_render_target;

typedef struct kinc_g4_compute_constant_location {
	kinc_g4_compute_constant_location_impl impl;
} kinc_g4_compute_constant_location;

typedef struct kinc_g4_compute_texture_unit {
	kinc_g4_compute_texture_unit_impl impl;
} kinc_g4_compute_texture_unit;

typedef struct kinc_g4_compute_shader {
	kinc_g4_compute_shader_impl impl;
} kinc_g4_compute_shader;

/// <summary>
/// Initialize a compute-shader from system-specific shader-data.
/// </summary>
/// <param name="shader">The shader-object to initialize</param>
/// <param name="source">A pointer to system-specific shader-data</param>
/// <param name="length">Length of the shader-data in bytes</param>
KINC_FUNC void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *source, int length);

/// <summary>
/// Destroy a shader-object
/// </summary>
/// <param name="shader">The shader-object to destroy</param>
KINC_FUNC void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader);

#ifndef KINC_KONG
/// <summary>
/// Finds the location of a constant/uniform inside of a shader.
/// </summary>
/// <param name="shader">The shader to look into</param>
/// <param name="name">The constant/uniform-name to look for</param>
/// <returns>The found constant-location</returns>
KINC_FUNC kinc_g4_compute_constant_location kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name);

/// <summary>
/// Finds a texture-unit inside of a shader.
/// </summary>
/// <param name="shader">The shader to look into</param>
/// <param name="name">The texture-name to look for</param>
/// <returns>The found texture-unit</returns>
KINC_FUNC kinc_g4_compute_texture_unit kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name);
#endif

#ifdef KORE_OPENGL
typedef struct kinc_shader_storage_buffer {
	kinc_compute_shader_storage_buffer_impl_t impl;
} kinc_shader_storage_buffer_t;

KINC_FUNC void kinc_shader_storage_buffer_init(kinc_shader_storage_buffer_t *buffer, int count, kinc_g4_vertex_data_t type);
KINC_FUNC void kinc_shader_storage_buffer_destroy(kinc_shader_storage_buffer_t *buffer);
KINC_FUNC int *kinc_shader_storage_buffer_lock(kinc_shader_storage_buffer_t *buffer);
KINC_FUNC void kinc_shader_storage_buffer_unlock(kinc_shader_storage_buffer_t *buffer);
KINC_FUNC int kinc_shader_storage_buffer_count(kinc_shader_storage_buffer_t *buffer);
KINC_FUNC void kinc_shader_storage_buffer_internal_set(kinc_shader_storage_buffer_t *buffer);
#endif

typedef enum kinc_g4_compute_access { KINC_G4_COMPUTE_ACCESS_READ, KINC_G4_COMPUTE_ACCESS_WRITE, KINC_G4_COMPUTE_ACCESS_READ_WRITE } kinc_g4_compute_access;

/// <summary>
/// Assigns a bool-value to a constant/uniform. The constant/uniform has to be declared as a bool in the shader.
/// </summary>
/// <param name="location">The location to set</param>
/// <param name="value">The value to set the location to</param>
KINC_FUNC void kinc_g4_compute_set_bool(kinc_g4_compute_constant_location location, bool value);

/// <summary>
/// Assigns an int-value to a constant/uniform. The constant/uniform has to be declared as an int in the shader.
/// </summary>
/// <param name="location">The location to set</param>
/// <param name="value">The value to set the location to</param>
KINC_FUNC void kinc_g4_compute_set_int(kinc_g4_compute_constant_location location, int value);

/// <summary>
/// Assigns a float-value to a constant/uniform. The constant/uniform has to be declared as a float in the shader.
/// </summary>
/// <param name="location">The location to set</param>
/// <param name="value">The value to set the location to</param>
KINC_FUNC void kinc_g4_compute_set_float(kinc_g4_compute_constant_location location, float value);

/// <summary>
/// Assigns two float-values to a constant/uniform. The constant/uniform has to be declared as a vec2 in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_float2(kinc_g4_compute_constant_location location, float value1, float value2);

/// <summary>
/// Assigns three float-values to a constant/uniform. The constant/uniform has to be declared as a vec3 in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_float3(kinc_g4_compute_constant_location location, float value1, float value2, float value3);

/// <summary>
/// Assigns four float-values to a constant/uniform. The constant/uniform has to be declared as a vec4 in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_float4(kinc_g4_compute_constant_location location, float value1, float value2, float value3, float value4);

/// <summary>
/// Assigns an array of float-values to a constant/uniform. The constant/uniform has to be declared as a float-array in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_floats(kinc_g4_compute_constant_location location, float *values, int count);

/// <summary>
/// Assigns a 4x4-matrix-value to a constant/uniform. The constant/uniform has to be declared as a mat4 in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_matrix4(kinc_g4_compute_constant_location location, kinc_matrix4x4_t *value);

/// <summary>
/// Assigns a 3x3-matrix-value to a constant/uniform. The constant/uniform has to be declared as a mat3 in the shader.
/// </summary>
/// <param name="location">The location to set</param>
KINC_FUNC void kinc_g4_compute_set_matrix3(kinc_g4_compute_constant_location location, kinc_matrix3x3_t *value);

#ifdef KORE_OPENGL
KINC_FUNC void kinc_g4_compute_set_buffer(kinc_shader_storage_buffer_t *buffer, int index);
#endif

/// <summary>
/// Assigns a texture to a texture-unit for direct access.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture(kinc_g4_compute_texture_unit unit, struct kinc_g4_texture *texture, kinc_g4_compute_access access);

/// <summary>
/// Assigns a render-target to a texture-unit for direct access.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *texture, kinc_g4_compute_access access);

/// <summary>
/// Assigns a texture to a texture-unit for samples access.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_sampled_texture(kinc_g4_compute_texture_unit unit, struct kinc_g4_texture *texture);

/// <summary>
/// Assigns a render-target to a texture-unit for samples access.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_sampled_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *target);

/// <summary>
/// Assigns the depth-component of a render-target to a texture-unit for samples access.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_sampled_depth_from_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *target);

/// <summary>
/// Assigns the mode for accessing a texture outside of the 0 to 1-range for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture_addressing(kinc_g4_compute_texture_unit unit, kinc_g4_texture_direction_t dir,
                                                      kinc_g4_texture_addressing_t addressing);

/// <summary>
/// Sets the magnification-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture_magnification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Sets the minification-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture_minification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Sets the mipmap-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture_mipmap_filter(kinc_g4_compute_texture_unit unit, kinc_g4_mipmap_filter_t filter);

/// <summary>
/// Assigns the mode for accessing a texture outside of the 0 to 1-range for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture3d_addressing(kinc_g4_compute_texture_unit unit, kinc_g4_texture_direction_t dir,
                                                        kinc_g4_texture_addressing_t addressing);

/// <summary>
/// Sets the magnification-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture3d_magnification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Sets the minification-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture3d_minification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Sets the mipmap-mode for a texture-unit.
/// </summary>
KINC_FUNC void kinc_g4_compute_set_texture3d_mipmap_filter(kinc_g4_compute_texture_unit unit, kinc_g4_mipmap_filter_t filter);

/// <summary>
/// Sets a shader for the next compute-run.
/// </summary>
/// <param name="shader">The shader to use</param>
KINC_FUNC void kinc_g4_compute_set_shader(kinc_g4_compute_shader *shader);

/// <summary>
/// Fire off a compute-run on x * y * z elements.
/// </summary>
/// <param name="x">The x-size for the compute-run</param>
/// <param name="y">The y-size for the compute-run</param>
/// <param name="z">The z-size for the compute-run</param>
KINC_FUNC void kinc_g4_compute(int x, int y, int z);

#ifdef __cplusplus
}
#endif
