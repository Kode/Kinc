#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/compute.h>
#ifdef KORE_OPENGL
#include <kinc/backend/graphics4/ShaderStorageBufferImpl.h>
#include <kinc/graphics4/vertexbuffer.h>
#endif
#include <kinc/graphics4/graphics.h>

/*! \file compute.h
    \brief Provides support for running compute-shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_texture;
struct kinc_g4_render_target;

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
KINC_FUNC kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name);

/// <summary>
/// Finds a texture-unit inside of a shader.
/// </summary>
/// <param name="shader">The shader to look into</param>
/// <param name="name">The texture-name to look for</param>
/// <returns>The found texture-unit</returns>
KINC_FUNC kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name);
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

#ifdef KORE_OPENGL
KINC_FUNC void kinc_g4_compute_set_buffer(kinc_shader_storage_buffer_t *buffer, int index);
#endif

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
