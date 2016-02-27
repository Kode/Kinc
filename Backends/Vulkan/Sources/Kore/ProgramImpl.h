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
	class Program;
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

		VkBuffer bufVertex;
		VkMemoryAllocateInfo mem_allocVertex;
		VkDeviceMemory memVertex;
		VkDescriptorBufferInfo buffer_infoVertex;

		VkBuffer bufFragment;
		VkMemoryAllocateInfo mem_allocFragment;
		VkDeviceMemory memFragment;
		VkDescriptorBufferInfo buffer_infoFragment;

		std::map<std::string, u32> vertexLocations;
		std::map<std::string, u32> fragmentLocations;
		std::map<std::string, u32> textureBindings;
	public:
		float uniformDataVertex[256];
		float uniformDataFragment[256];
		static Program* current;
	};

	class ConstantLocationImpl {
	public:
		int location;
		bool vertex;
	};
}
