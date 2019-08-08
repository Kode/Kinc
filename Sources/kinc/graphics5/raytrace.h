#pragma once

#ifdef KORE_DXR

#include <Kore/RayTraceImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_raytrace_pipeline {
	kinc_g5_constant_buffer_t *_constant_buffer;
	kinc_raytrace_pipeline_impl_t impl;
} kinc_raytrace_pipeline_t;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size, kinc_g5_constant_buffer_t *constant_buffer);

typedef struct kinc_raytrace_acceleration_structure {
	kinc_raytrace_acceleration_structure_impl_t impl;
} kinc_raytrace_acceleration_structure_t;

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb, kinc_g5_index_buffer_t *ib);

typedef struct kinc_raytrace_target {
	int _width;
	int _height;
	kinc_raytrace_target_impl_t impl;
} kinc_raytrace_target_t;

void kinc_raytrace_target_init(kinc_raytrace_target_t *target, int width, int height);

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *accel);
void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *pipeline);
void kinc_raytrace_set_target(kinc_raytrace_target_t *output);
void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list);
void kinc_raytrace_copy_target(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t *render_target, kinc_raytrace_target_t *output);

#ifdef __cplusplus
}
#endif

#endif
