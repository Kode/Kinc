#include "commandlist_functions.h"

#include "vulkanunit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

#include <kope/vulkan/texture_functions.h>

#include "pipeline_structs.h"

#include <kope/util/align.h>

#include <assert.h>

void kope_vulkan_command_list_destroy(kope_g5_command_list *list) {
	vkFreeCommandBuffers(list->vulkan.device, list->vulkan.command_pool, 1, &list->vulkan.command_buffer);
}

void kope_vulkan_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	const kope_g5_texture *texture = parameters->color_attachments[0].texture.texture;

	const VkClearValue clear_value = {
	    .color =
	        {
	            .float32 = {0.0f, 0.0f, 0.0f, 1.0f},
	        },
	    .depthStencil =
	        {
	            .depth = 1.0f,
	            .stencil = 0,
	        },
	};

	const VkRenderingAttachmentInfo color_attachment_info = {
	    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
	    .pNext = NULL,
	    .imageView = texture->vulkan.image_view,
	    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	    .resolveMode = VK_RESOLVE_MODE_NONE,
	    .resolveImageView = VK_NULL_HANDLE,
	    .resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL,
	    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	    .clearValue = clear_value,
	};

	const VkRect2D render_area = {
	    .offset =
	        {
	            .x = 0,
	            .y = 0,
	        },
	    .extent =
	        {
	            .width = texture->vulkan.width,
	            .height = texture->vulkan.height,
	        },
	};

	const VkRenderingInfo rendering_info = {
	    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
	    .pNext = NULL,
	    .flags = 0,
	    .renderArea = render_area,
	    .layerCount = 1,
	    .viewMask = 0,
	    .colorAttachmentCount = 1,
	    .pColorAttachments = &color_attachment_info,
	    .pDepthAttachment = VK_NULL_HANDLE,
	    .pStencilAttachment = VK_NULL_HANDLE,
	};

	vkCmdBeginRendering(list->vulkan.command_buffer, &rendering_info);
}

void kope_vulkan_command_list_end_render_pass(kope_g5_command_list *list) {
	vkCmdEndRendering(list->vulkan.command_buffer);
}

void kope_vulkan_command_list_present(kope_g5_command_list *list) {
	list->vulkan.presenting = true;
}

void kope_vulkan_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                               uint64_t size) {}

void kope_vulkan_command_list_set_vertex_buffer(kope_g5_command_list *list, uint32_t slot, kope_vulkan_buffer *buffer, uint64_t offset, uint64_t size,
                                                uint64_t stride) {}

void kope_vulkan_command_list_set_render_pipeline(kope_g5_command_list *list, kope_vulkan_render_pipeline *pipeline) {
	vkCmdBindPipeline(list->vulkan.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
}

void kope_vulkan_command_list_draw(kope_g5_command_list *list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
}

void kope_vulkan_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                           uint32_t first_instance) {}

void kope_vulkan_command_list_set_descriptor_table(kope_g5_command_list *list, uint32_t table_index, kope_vulkan_descriptor_set *set,
                                                   kope_g5_buffer **dynamic_buffers, uint32_t *dynamic_offsets, uint32_t *dynamic_sizes) {}

void kope_vulkan_command_list_set_root_constants(kope_g5_command_list *list, uint32_t table_index, const void *data, size_t data_size) {}

void kope_vulkan_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset, kope_g5_buffer *destination,
                                                    uint64_t destination_offset, uint64_t size) {}

void kope_vulkan_command_list_copy_buffer_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_buffer *source,
                                                     const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                     uint32_t depth_or_array_layers) {}

void kope_vulkan_command_list_copy_texture_to_buffer(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                     const kope_g5_image_copy_buffer *destination, uint32_t width, uint32_t height,
                                                     uint32_t depth_or_array_layers) {}

void kope_vulkan_command_list_copy_texture_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                      const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                      uint32_t depth_or_array_layers) {}

void kope_vulkan_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size) {}

void kope_vulkan_command_list_set_compute_pipeline(kope_g5_command_list *list, kope_vulkan_compute_pipeline *pipeline) {}

void kope_vulkan_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {}

void kope_vulkan_command_list_prepare_raytracing_volume(kope_g5_command_list *list, kope_g5_raytracing_volume *volume) {}

void kope_vulkan_command_list_prepare_raytracing_hierarchy(kope_g5_command_list *list, kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_vulkan_command_list_update_raytracing_hierarchy(kope_g5_command_list *list, kinc_matrix4x4_t *volume_transforms, uint32_t volumes_count,
                                                          kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_vulkan_command_list_set_ray_pipeline(kope_g5_command_list *list, kope_vulkan_ray_pipeline *pipeline) {}

void kope_vulkan_command_list_trace_rays(kope_g5_command_list *list, uint32_t width, uint32_t height, uint32_t depth) {}

void kope_vulkan_command_list_set_viewport(kope_g5_command_list *list, float x, float y, float width, float height, float min_depth, float max_depth) {}

void kope_vulkan_command_list_set_scissor_rect(kope_g5_command_list *list, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

void kope_vulkan_command_list_set_blend_constant(kope_g5_command_list *list, kope_g5_color color) {}

void kope_vulkan_command_list_set_stencil_reference(kope_g5_command_list *list, uint32_t reference) {}

void kope_vulkan_command_list_set_name(kope_g5_command_list *list, const char *name) {
	const VkDebugMarkerObjectNameInfoEXT name_info = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
	    .pNext = NULL,
	    .objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
	    .object = (uint64_t)list->vulkan.command_buffer,
	    .pObjectName = name,
	};

	vulkan_DebugMarkerSetObjectNameEXT(list->vulkan.device, &name_info);
}

void kope_vulkan_command_list_push_debug_group(kope_g5_command_list *list, const char *name) {
	const VkDebugMarkerMarkerInfoEXT marker_info = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
	    .pNext = NULL,
	    .pMarkerName = name,
	    .color = {0.0f, 0.0f, 0.0f, 1.0f},
	};

	vulkan_CmdDebugMarkerBeginEXT(list->vulkan.command_buffer, &marker_info);
}

void kope_vulkan_command_list_pop_debug_group(kope_g5_command_list *list) {
	vulkan_CmdDebugMarkerEndEXT(list->vulkan.command_buffer);
}

void kope_vulkan_command_list_insert_debug_marker(kope_g5_command_list *list, const char *name) {
	const VkDebugMarkerMarkerInfoEXT marker_info = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
	    .pNext = NULL,
	    .pMarkerName = name,
	    .color = {0.0f, 0.0f, 0.0f, 1.0f},
	};

	vulkan_CmdDebugMarkerInsertEXT(list->vulkan.command_buffer, &marker_info);
}

void kope_vulkan_command_list_begin_occlusion_query(kope_g5_command_list *list, uint32_t query_index) {}

void kope_vulkan_command_list_end_occlusion_query(kope_g5_command_list *list) {}

void kope_vulkan_command_list_resolve_query_set(kope_g5_command_list *list, kope_g5_query_set *query_set, uint32_t first_query, uint32_t query_count,
                                                kope_g5_buffer *destination, uint64_t destination_offset) {}

void kope_vulkan_command_list_draw_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset, uint32_t max_draw_count,
                                            kope_g5_buffer *count_buffer, uint64_t count_offset) {}

void kope_vulkan_command_list_draw_indexed_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset,
                                                    uint32_t max_draw_count, kope_g5_buffer *count_buffer, uint64_t count_offset) {}

void kope_vulkan_command_list_compute_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset) {}

void kope_vulkan_command_list_queue_buffer_access(kope_g5_command_list *list, kope_g5_buffer *buffer, uint32_t offset, uint32_t size) {}

void kope_vulkan_command_list_queue_descriptor_set_access(kope_g5_command_list *list, kope_vulkan_descriptor_set *descriptor_set) {}
