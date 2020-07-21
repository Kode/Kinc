#pragma once

#include <vulkan/vulkan.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef RegisterClass
#undef RegisterClass
#endif

struct kinc_g5_shader;

#define KINC_INTERNAL_NAMED_NUMBER_COUNT 32

typedef struct {
	char name[256];
	uint32_t number;
} kinc_internal_named_number;

typedef struct PipelineState5Impl_s {
	const char** textures;
	int* textureValues;
	int textureCount;

	VkPipeline pipeline;
	VkPipelineCache pipelineCache;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	kinc_internal_named_number vertexLocations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number fragmentLocations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number textureBindings[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number vertexOffsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number fragmentOffsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];

	VkPipelineLayout pipeline_layout;
} PipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;
