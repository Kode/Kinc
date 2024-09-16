#ifndef KOPE_D3D12_COMMANDLIST_FUNCTIONS_HEADER
#define KOPE_D3D12_COMMANDLIST_FUNCTIONS_HEADER

#include <kope/graphics5/commandlist.h>

#include "descriptorset_structs.h"
#include "pipeline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_command_list_copy_buffer_to_texture(kope_g5_command_list *list, kope_g5_buffer *source, kope_g5_texture *destination, kope_uint3 size);

void kope_d3d12_command_list_copy_texture_to_texture(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_texture *destination, kope_uint3 size);

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters);

void kope_d3d12_command_list_end_render_pass(kope_g5_command_list *list);

void kope_d3d12_command_list_present(kope_g5_command_list *list);

void kope_d3d12_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                              uint64_t size);

void kope_d3d12_command_list_set_vertex_buffer(kope_g5_command_list *list, uint32_t slot, kope_d3d12_buffer *buffer, uint64_t offset, uint64_t size,
                                               uint64_t stride);

void kope_d3d12_command_list_set_render_pipeline(kope_g5_command_list *list, kope_d3d12_render_pipeline *pipeline);

void kope_d3d12_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                          uint32_t first_instance);

void kope_d3d12_command_list_set_descriptor_table(kope_g5_command_list *list, uint32_t table_index, kope_d3d12_descriptor_set *set);

void kope_d3d12_command_list_set_compute_pipeline(kope_g5_command_list *list, kope_d3d12_compute_pipeline *pipeline);

void kope_d3d12_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z);

void kope_d3d12_command_list_prepare_raytracing_volume(kope_g5_command_list *list, kope_g5_raytracing_volume *volume);

void kope_d3d12_command_list_prepare_raytracing_hierarchy(kope_g5_command_list *list, kope_g5_raytracing_hierarchy *hierarchy);

void kope_d3d12_command_list_update_raytracing_hierarchy(kope_g5_command_list *list, kinc_matrix4x4_t *volume_transforms, uint32_t volumes_count,
                                                         kope_g5_raytracing_hierarchy *hierarchy);

void kope_d3d12_command_list_set_ray_pipeline(kope_g5_command_list *list, kope_d3d12_ray_pipeline *pipeline);

void kope_d3d12_command_list_trace_rays(kope_g5_command_list *list);

#ifdef __cplusplus
}
#endif

#endif
