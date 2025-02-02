#include "commandlist.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/commandlist_functions.h>
#endif

#ifdef KOPE_METAL
#include <kope/metal/commandlist_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/commandlist_functions.h>
#endif

#include <assert.h>

void kope_g5_command_list_destroy(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_destroy, list);
}

void kope_g5_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset, kope_g5_buffer *destination,
                                                uint64_t destination_offset, uint64_t size) {
	KOPE_G5_CALL6(command_list_copy_buffer_to_buffer, list, source, source_offset, destination, destination_offset, size);
}

void kope_g5_command_list_copy_buffer_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_buffer *source,
                                                 const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                 uint32_t depth_or_array_layers) {
#ifdef KOPE_G5_VALIDATION
	assert(source->offset % 512 == 0); // D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
#endif
	KOPE_G5_CALL6(command_list_copy_buffer_to_texture, list, source, destination, width, height, depth_or_array_layers);
}

void kope_g5_command_list_copy_texture_to_buffer(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                 const kope_g5_image_copy_buffer *destination, uint32_t width, uint32_t height,
                                                 uint32_t depth_or_array_layers) {
	KOPE_G5_CALL6(command_list_copy_texture_to_buffer, list, source, destination, width, height, depth_or_array_layers);
}

void kope_g5_command_list_copy_texture_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                  const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                  uint32_t depth_or_array_layers) {
#ifdef KOPE_G5_VALIDATION
	assert(source->texture->validation_format == destination->texture->validation_format);
#endif
	KOPE_G5_CALL6(command_list_copy_texture_to_texture, list, source, destination, width, height, depth_or_array_layers);
}

void kope_g5_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size) {
	KOPE_G5_CALL4(command_list_clear_buffer, list, buffer, offset, size);
}

void kope_g5_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	KOPE_G5_CALL2(command_list_begin_render_pass, list, parameters);
}

void kope_g5_command_list_end_render_pass(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_end_render_pass, list);
}

void kope_g5_command_list_present(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_present, list);
}

void kope_g5_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                           uint64_t size) {
	KOPE_G5_CALL5(command_list_set_index_buffer, list, buffer, index_format, offset, size);
}

void kope_g5_command_list_draw(kope_g5_command_list *list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
	KOPE_G5_CALL5(command_list_draw, list, vertex_count, instance_count, first_vertex, first_instance);
}

void kope_g5_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                       uint32_t first_instance) {
	KOPE_G5_CALL6(command_list_draw_indexed, list, index_count, instance_count, first_index, base_vertex, first_instance);
}

void kope_g5_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {
	KOPE_G5_CALL4(command_list_compute, list, workgroup_count_x, workgroup_count_y, workgroup_count_z);
}

void kope_g5_command_list_prepare_raytracing_volume(kope_g5_command_list *list, kope_g5_raytracing_volume *volume) {
	KOPE_G5_CALL2(command_list_prepare_raytracing_volume, list, volume);
}
void kope_g5_command_list_prepare_raytracing_hierarchy(kope_g5_command_list *list, kope_g5_raytracing_hierarchy *hierarchy) {
	KOPE_G5_CALL2(command_list_prepare_raytracing_hierarchy, list, hierarchy);
}

void kope_g5_command_list_update_raytracing_hierarchy(kope_g5_command_list *list, kinc_matrix4x4_t *volume_transforms, uint32_t volumes_count,
                                                      kope_g5_raytracing_hierarchy *hierarchy) {
	KOPE_G5_CALL4(command_list_update_raytracing_hierarchy, list, volume_transforms, volumes_count, hierarchy);
}

void kope_g5_command_list_trace_rays(kope_g5_command_list *list, uint32_t width, uint32_t height, uint32_t depth) {
	KOPE_G5_CALL4(command_list_trace_rays, list, width, height, depth);
}

void kope_g5_command_list_set_viewport(kope_g5_command_list *list, float x, float y, float width, float height, float min_depth, float max_depth) {
	KOPE_G5_CALL7(command_list_set_viewport, list, x, y, width, height, min_depth, max_depth);
}

void kope_g5_command_list_set_scissor_rect(kope_g5_command_list *list, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	KOPE_G5_CALL5(command_list_set_scissor_rect, list, x, y, width, height);
}

void kope_g5_command_list_set_blend_constant(kope_g5_command_list *list, kope_g5_color color) {
	KOPE_G5_CALL2(command_list_set_blend_constant, list, color);
}

void kope_g5_command_list_set_stencil_reference(kope_g5_command_list *list, uint32_t reference) {
	KOPE_G5_CALL2(command_list_set_stencil_reference, list, reference);
}

void kope_g5_command_list_set_name(kope_g5_command_list *list, const char *name) {
	KOPE_G5_CALL2(command_list_set_name, list, name);
}

void kope_g5_command_list_push_debug_group(kope_g5_command_list *list, const char *name) {
	KOPE_G5_CALL2(command_list_push_debug_group, list, name);
}

void kope_g5_command_list_pop_debug_group(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_pop_debug_group, list);
}

void kope_g5_command_list_insert_debug_marker(kope_g5_command_list *list, const char *name) {
	KOPE_G5_CALL2(command_list_insert_debug_marker, list, name);
}

void kope_g5_command_list_begin_occlusion_query(kope_g5_command_list *list, uint32_t query_index) {
	KOPE_G5_CALL2(command_list_begin_occlusion_query, list, query_index);
}

void kope_g5_command_list_end_occlusion_query(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_end_occlusion_query, list);
}

void kope_g5_command_list_resolve_query_set(kope_g5_command_list *list, kope_g5_query_set *query_set, uint32_t first_query, uint32_t query_count,
                                            kope_g5_buffer *destination, uint64_t destination_offset) {
	KOPE_G5_CALL6(command_list_resolve_query_set, list, query_set, first_query, query_count, destination, destination_offset);
}

void kope_g5_command_list_draw_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset, uint32_t max_draw_count,
                                        kope_g5_buffer *count_buffer, uint64_t count_offset) {
	KOPE_G5_CALL6(command_list_draw_indirect, list, indirect_buffer, indirect_offset, max_draw_count, count_buffer, count_offset);
}

void kope_g5_command_list_draw_indexed_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset, uint32_t max_draw_count,
                                                kope_g5_buffer *count_buffer, uint64_t count_offset) {
	KOPE_G5_CALL6(command_list_draw_indexed_indirect, list, indirect_buffer, indirect_offset, max_draw_count, count_buffer, count_offset);
}

void kope_g5_command_list_compute_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset) {
	KOPE_G5_CALL3(command_list_compute_indirect, list, indirect_buffer, indirect_offset);
}
