#pragma once

#ifdef KORE_RAYTRACE

#include <kinc/global.h>

#include <kinc/backend/graphics5/raytrace.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_command_list;
struct kinc_g5_constant_buffer;
struct kinc_g5_index_buffer;
struct kinc_g5_render_target;
struct kinc_g5_texture;
struct kinc_g5_vertex_buffer;

typedef struct kinc_raytrace_pipeline {
	struct kinc_g5_constant_buffer *_constant_buffer;
	kinc_raytrace_pipeline_impl_t impl;
} kinc_raytrace_pipeline_t;

KINC_FUNC void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, struct kinc_g5_command_list *command_list, void *ray_shader, int ray_shader_size,
                                           struct kinc_g5_constant_buffer *constant_buffer);
KINC_FUNC void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline);

typedef struct kinc_raytrace_acceleration_structure {
	kinc_raytrace_acceleration_structure_impl_t impl;
} kinc_raytrace_acceleration_structure_t;

KINC_FUNC void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, struct kinc_g5_command_list *command_list,
                                                         struct kinc_g5_vertex_buffer *vb, struct kinc_g5_index_buffer *ib);
KINC_FUNC void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel);

KINC_FUNC void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *accel);
KINC_FUNC void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *pipeline);
KINC_FUNC void kinc_raytrace_set_target(struct kinc_g5_texture *output);
KINC_FUNC void kinc_raytrace_dispatch_rays(struct kinc_g5_command_list *command_list);
KINC_FUNC void kinc_raytrace_copy(struct kinc_g5_command_list *command_list, struct kinc_g5_render_target *target, struct kinc_g5_texture *source);

#ifdef __cplusplus
}
#endif

#endif
