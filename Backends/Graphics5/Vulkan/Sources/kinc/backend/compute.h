#pragma once

#include <kinc/backend/graphics5/ShaderHash.h>

#include "graphics5/MiniVulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_compute_texture_unit_impl_t;

typedef struct {
	uint32_t hash;
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_internal_shader_constant_t;

typedef struct {
	kinc_compute_internal_shader_constant_t constants[64];
	int constantsSize;
	kinc_internal_hash_index_t attributes[64];
	kinc_internal_hash_index_t textures[64];
	uint8_t *data;
	int length;

	VkDescriptorSet descriptor_set;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
} kinc_compute_shader_impl_t;

#ifdef __cplusplus
}
#endif
