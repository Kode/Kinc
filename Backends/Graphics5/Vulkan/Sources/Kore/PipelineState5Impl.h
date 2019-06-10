#pragma once

#include <map>
#include <string>

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

typedef struct PipelineState5Impl_s {
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;
	struct kinc_g5_shader *geometryShader;
	struct kinc_g5_shader *tessEvalShader;
	struct kinc_g5_shader *tessControlShader;

	const char** textures;
	int* textureValues;
	int textureCount;

	VkPipeline pipeline;
	VkPipelineCache pipelineCache;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	std::map<std::string, uint32_t> vertexLocations;
	std::map<std::string, uint32_t> fragmentLocations;
	std::map<std::string, uint32_t> textureBindings;
	std::map<std::string, uint32_t> vertexOffsets;
	std::map<std::string, uint32_t> fragmentOffsets;

	VkPipelineLayout pipeline_layout;

	VkDescriptorSetLayout desc_layout;

	//static Graphics5::PipelineState* current;
} PipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;
