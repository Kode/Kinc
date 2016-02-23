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

namespace Kore {
	class Shader;

	class ProgramImpl {
	protected:
		uint programId;
		Shader* vertexShader;
		Shader* fragmentShader;
		Shader* geometryShader;
		Shader* tesselationControlShader;
		Shader* tesselationEvaluationShader;

		ProgramImpl();
		virtual ~ProgramImpl();

		const char** textures;
		int* textureValues;
		int textureCount;

		VkDescriptorSetLayout desc_layout;
		VkPipelineLayout pipeline_layout;
		VkPipeline pipeline;
		VkPipelineCache pipelineCache;
		VkShaderModule vert_shader_module;
		VkShaderModule frag_shader_module;

		VkBuffer buf;
		VkMemoryAllocateInfo mem_alloc;
		VkDeviceMemory mem;
		VkDescriptorBufferInfo buffer_info;

		std::map<std::string, u32> vertexLocations;
	};

	class ConstantLocationImpl {
	public:
		int location;
	};
}
