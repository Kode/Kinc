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

KOPE_FUNC void kope_g5_command_list_pushDebugGroup(kope_g5_command_list *list, const char *name);

KOPE_FUNC void kope_g5_command_list_popDebugGroup(kope_g5_command_list *list);

KOPE_FUNC void kope_g5_command_list_insertDebugMarker(kope_g5_command_list *list, const char *name);

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

KOPE_FUNC void kope_g5_command_list_begin_render_pass(kope_g5_command_list *list, kope_g5_render_pass_parameters parameters);

KOPE_FUNC void kope_g5_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset,
                                                          kope_g5_buffer *destination, uint64_t destination_offset, uint64_t size);

KOPE_FUNC void copyBufferToTexturekope_g5_command_list_(kope_g5_command_list *list, kope_g5_buffer *source, kope_g5_texture *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_copy_texture_to_buffer(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_buffer *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_copy_texture_to_texture(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_texture *destination, kope_uint3 size);

KOPE_FUNC void kope_g5_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size);

// KOPE_FUNC void kope_g5_command_list_resolve_query_set(kope_g5_command_list *list, GPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount,
//                                                     kope_g5_buffer *destination, uint64_t destinationOffset);

KOPE_FUNC void kope_g5_command_list_finish(kope_g5_command_list *list);

#ifdef __cplusplus
}
#endif

#endif
