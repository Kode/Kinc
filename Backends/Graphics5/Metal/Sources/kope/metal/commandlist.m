#include "commandlist_functions.h"

#include "metalunit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

#include <kope/metal/texture_functions.h>

#include "pipeline_structs.h"

#include <kope/util/align.h>

#include <assert.h>

void kope_metal_command_list_destroy(kope_g5_command_list *list) {}

void kope_metal_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	id<MTLTexture> texture = (__bridge id<MTLTexture>)parameters->color_attachments[0].texture.texture->metal.texture;
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
	renderPassDescriptor.depthAttachment.clearDepth = 1;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = nil; // depthTexture;
	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
	renderPassDescriptor.stencilAttachment.texture = nil; // depthTexture;

	id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)list->metal.command_buffer;
	list->metal.render_command_encoder = (__bridge_retained void *)[command_buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void kope_metal_command_list_end_render_pass(kope_g5_command_list *list) {
	id<MTLRenderCommandEncoder> render_command_encoder = (__bridge id<MTLRenderCommandEncoder>)list->metal.render_command_encoder;
	[render_command_encoder endEncoding];
	list->metal.render_command_encoder = NULL;
}

void kope_metal_command_list_present(kope_g5_command_list *list) {
	//[command_buffer presentDrawable:drawable];
}

void kope_metal_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                              uint64_t size) {}

void kope_metal_command_list_set_vertex_buffer(kope_g5_command_list *list, uint32_t slot, kope_metal_buffer *buffer, uint64_t offset, uint64_t size,
                                               uint64_t stride) {}

void kope_metal_command_list_set_render_pipeline(kope_g5_command_list *list, kope_metal_render_pipeline *pipeline) {}

void kope_metal_command_list_draw(kope_g5_command_list *list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {}

void kope_metal_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                          uint32_t first_instance) {}

void kope_metal_command_list_set_descriptor_table(kope_g5_command_list *list, uint32_t table_index, kope_metal_descriptor_set *set,
                                                  kope_g5_buffer **dynamic_buffers, uint32_t *dynamic_offsets, uint32_t *dynamic_sizes) {}

void kope_metal_command_list_set_root_constants(kope_g5_command_list *list, uint32_t table_index, const void *data, size_t data_size) {}

void kope_metal_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset, kope_g5_buffer *destination,
                                                   uint64_t destination_offset, uint64_t size) {}

void kope_metal_command_list_copy_buffer_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_buffer *source,
                                                    const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                    uint32_t depth_or_array_layers) {}

void kope_metal_command_list_copy_texture_to_buffer(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                    const kope_g5_image_copy_buffer *destination, uint32_t width, uint32_t height,
                                                    uint32_t depth_or_array_layers) {}

void kope_metal_command_list_copy_texture_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                     const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                     uint32_t depth_or_array_layers) {}

void kope_metal_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size) {}

void kope_metal_command_list_set_compute_pipeline(kope_g5_command_list *list, kope_metal_compute_pipeline *pipeline) {}

void kope_metal_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {}

void kope_metal_command_list_prepare_raytracing_volume(kope_g5_command_list *list, kope_g5_raytracing_volume *volume) {}

void kope_metal_command_list_prepare_raytracing_hierarchy(kope_g5_command_list *list, kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_metal_command_list_update_raytracing_hierarchy(kope_g5_command_list *list, kinc_matrix4x4_t *volume_transforms, uint32_t volumes_count,
                                                         kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_metal_command_list_set_ray_pipeline(kope_g5_command_list *list, kope_metal_ray_pipeline *pipeline) {}

void kope_metal_command_list_trace_rays(kope_g5_command_list *list, uint32_t width, uint32_t height, uint32_t depth) {}

void kope_metal_command_list_set_viewport(kope_g5_command_list *list, float x, float y, float width, float height, float min_depth, float max_depth) {}

void kope_metal_command_list_set_scissor_rect(kope_g5_command_list *list, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

void kope_metal_command_list_set_blend_constant(kope_g5_command_list *list, kope_g5_color color) {}

void kope_metal_command_list_set_stencil_reference(kope_g5_command_list *list, uint32_t reference) {}

void kope_metal_command_list_set_name(kope_g5_command_list *list, const char *name) {}

void kope_metal_command_list_push_debug_group(kope_g5_command_list *list, const char *name) {}

void kope_metal_command_list_pop_debug_group(kope_g5_command_list *list) {}

void kope_metal_command_list_insert_debug_marker(kope_g5_command_list *list, const char *name) {}

void kope_metal_command_list_begin_occlusion_query(kope_g5_command_list *list, uint32_t query_index) {}

void kope_metal_command_list_end_occlusion_query(kope_g5_command_list *list) {}

void kope_metal_command_list_resolve_query_set(kope_g5_command_list *list, kope_g5_query_set *query_set, uint32_t first_query, uint32_t query_count,
                                               kope_g5_buffer *destination, uint64_t destination_offset) {}

void kope_metal_command_list_draw_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset, uint32_t max_draw_count,
                                           kope_g5_buffer *count_buffer, uint64_t count_offset) {}

void kope_metal_command_list_draw_indexed_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset,
                                                   uint32_t max_draw_count, kope_g5_buffer *count_buffer, uint64_t count_offset) {}

void kope_metal_command_list_compute_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset) {}

void kope_metal_command_list_queue_buffer_access(kope_g5_command_list *list, kope_g5_buffer *buffer, uint32_t offset, uint32_t size) {}

void kope_metal_command_list_queue_descriptor_set_access(kope_g5_command_list *list, kope_metal_descriptor_set *descriptor_set) {}
