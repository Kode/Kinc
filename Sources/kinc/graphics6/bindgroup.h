#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics6/bindgroup.h>

#include "buffer.h"
#include "sampler.h"
#include "shader.h"
#include "texture.h"

/*! \file bindgroup.h
    \brief Provides functions for setting up and using bind groups.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g6_bind_group_layout {
	kinc_g6_bind_group_layout_impl_t impl;
} kinc_g6_bind_group_layout_t;
typedef struct kinc_g6_bind_group {
	kinc_g6_bind_group_impl_t impl;
} kinc_g6_bind_group_t;

typedef enum kinc_g6_binding_type {
	KINC_G6_BINDING_TYPE_SAMPLER,
	KINC_G6_BINDING_TYPE_TEXTURE,
	KINC_G6_BINDING_TYPE_STORAGE_TEXTURE,
	KINC_G6_BINDING_TYPE_UNIFORM_BUFFER,
	KINC_G6_BINDING_TYPE_STORAGE_BUFFER,
	KINC_G6_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC,
	KINC_G6_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC
} kinc_g6_binding_type_t;

typedef struct kinc_g6_bind_group_layout_entry {
	uint32_t binding;
	kinc_g6_shader_stage_flags_t visibility;
	kinc_g6_binding_type_t type;
} kinc_g6_bind_group_layout_entry_t;

typedef struct kinc_g6_bind_group_layout_descriptor {
	int entry_count;
	kinc_g6_bind_group_layout_entry_t *entries;
} kinc_g6_bind_group_layout_descriptor_t;

KINC_FUNC void kinc_g6_bind_group_layout_init(kinc_g6_bind_group_layout_t *layout,const kinc_g6_bind_group_layout_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_bind_group_layout_destroy(kinc_g6_bind_group_layout_t *layout);

typedef struct kinc_g6_bind_group_entry {
	uint32_t binding;
	union {
		struct kinc_g6_texture_view *texture;
		struct kinc_g6_sampler *sampler;
		struct {
			kinc_g6_buffer_t *buffer;
			uint64_t offset;
			uint64_t size;
		} buffer;
	};
} kinc_g6_bind_group_entry_t;

typedef struct kinc_g6_bind_group_descriptor {
	kinc_g6_bind_group_layout_t *layout;
	int entry_count;
	struct kinc_g6_bind_group_entry *entries;
} kinc_g6_bind_group_descriptor_t;

KINC_FUNC void kinc_g6_bind_group_init(kinc_g6_bind_group_t *group, const kinc_g6_bind_group_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_bind_group_destroy(kinc_g6_bind_group_t *group);
#ifdef __cplusplus
}
#endif