#include "commandlist.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/commandlist_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/commandlist_functions.h>
#endif

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

void kope_g5_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                       uint32_t first_instance) {
	KOPE_G5_CALL6(command_list_draw_indexed, list, index_count, instance_count, first_index, base_vertex, first_instance);
}

void kope_g5_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {
	KOPE_G5_CALL4(command_list_compute, list, workgroup_count_x, workgroup_count_y, workgroup_count_z);
}
