#ifndef KOPE_G5_COMMANDLIST_HEADER
#define KOPE_G5_COMMANDLIST_HEADER

#include <kope/global.h>

#include "api.h"
#include "buffer.h"
#include "texture.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/commandlist_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/commandlist_structs.h>
#endif

#include <kinc/math/vector.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_command_list {
	KOPE_G5_IMPL(command_list);
} kope_g5_command_list;

KOPE_FUNC void kope_g5_command_list_destroy(kope_g5_command_list *list);

KOPE_FUNC void kope_g5_command_list_set_name(kope_g5_command_list *list, const char *name);

KOPE_FUNC void kope_g5_command_list_push_debug_group(kope_g5_command_list *list, const char *name);

KOPE_FUNC void kope_g5_command_list_pop_debug_group(kope_g5_command_list *list);

KOPE_FUNC void kope_g5_command_list_insert_debug_marker(kope_g5_command_list *list, const char *name);

typedef enum kope_g5_load_op { KOPE_G5_LOAD_OP_LOAD, KOPE_G5_LOAD_OP_CLEAR } kope_g5_load_op;

typedef enum kope_g5_store_op { KOPE_G5_STORE_OP_STORE, KOPE_G5_STORE_OP_DISCARD } kope_g5_store_op;

typedef struct kope_g5_color {
	float r;
	float g;
	float b;
	float a;
} kope_g5_color;

typedef struct kope_g5_render_pass_color_attachment {
	kope_g5_texture *texture;
	uint32_t depth_slice;
	kope_g5_texture *resolve_target;
	kope_g5_color clear_value;
	kope_g5_load_op load_op;
	kope_g5_store_op store_op;
} kope_g5_render_pass_color_attachment;

typedef struct kope_g5_render_pass_depth_stencil_attachment {
	kope_g5_texture *texture;
	float depth_clear_value;
	kope_g5_load_op depth_load_op;
	kope_g5_store_op depth_store_op;
	bool depth_read_only;
	uint32_t stencil_clear_value;
	kope_g5_load_op stencil_load_op;
	kope_g5_store_op stencil_store_op;
	bool stencil_read_only;
} kope_g5_render_pass_depth_stencil_attachment;

typedef struct kope_g5_render_pass_parameters {
	kope_g5_render_pass_color_attachment color_attachments[8];
	kope_g5_render_pass_depth_stencil_attachment depth_stencil_attachments[8];
	// GPUQuerySet occlusionQuerySet;
	// GPURenderPassTimestampWrites timestampWrites;
} kope_g5_render_pass_parameters;

KOPE_FUNC void kope_g5_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters);

KOPE_FUNC void kope_g5_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset,
                                                          kope_g5_buffer *destination, uint64_t destination_offset, uint64_t size);

KOPE_FUNC void kope_g5_command_list_copy_buffer_to_texture(kope_g5_command_list *list, kope_g5_buffer *source, kope_g5_texture *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_copy_texture_to_buffer(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_buffer *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_copy_texture_to_texture(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_texture *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size);

// KOPE_FUNC void kope_g5_command_list_resolve_query_set(kope_g5_command_list *list, GPUQuerySet query_set, uint32_t first_query, uint32_t query_count,
//                                                     kope_g5_buffer *destination, uint64_t destination_offset);

typedef enum kope_g5_index_format { KOPE_G5_INDEX_FORMAT_UINT16, KOPE_G5_INDEX_FORMAT_UINT32 } kope_g5_index_format;

KOPE_FUNC void kope_g5_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                                     uint64_t size);

KOPE_FUNC void kope_g5_command_list_draw(kope_g5_command_list *list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
                                         uint32_t first_instance);

KOPE_FUNC void kope_g5_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                                                 int32_t base_vertex, uint32_t first_instance);

KOPE_FUNC void kope_g5_command_list_draw_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset);

KOPE_FUNC void kope_g5_command_list_draw_indexed_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset);

KOPE_FUNC void kope_g5_command_list_set_viewport(kope_g5_command_list *list, float x, float y, float width, float height, float min_depth, float max_depth);

KOPE_FUNC void kope_g5_command_list_set_scissor_rect(kope_g5_command_list *list, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

KOPE_FUNC void kope_g5_command_list_set_blend_constant(kope_g5_command_list *list, kope_g5_color color);

KOPE_FUNC void kope_g5_command_list_set_stencil_reference(kope_g5_command_list *list, uint32_t reference);

KOPE_FUNC void kope_g5_command_list_begin_occlusion_query(kope_g5_command_list *list, uint32_t query_index);

KOPE_FUNC void kope_g5_command_list_end_occlusion_query(kope_g5_command_list *list);

KOPE_FUNC void kope_g5_command_list_end_render_pass(kope_g5_command_list *list);

KOPE_FUNC void kope_g5_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z);

KOPE_FUNC void kope_g5_command_list_compute_indirect(kope_g5_command_list *list, kope_g5_buffer *indirect_buffer, uint64_t indirect_offset);

KOPE_FUNC void kope_g5_command_list_present(kope_g5_command_list *list);

#ifdef __cplusplus
}
#endif

#endif
