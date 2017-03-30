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
	namespace Graphics5 {
		class Program;
		class Shader;
	}

	class Program5Impl {
	protected:
		uint programId;
		Graphics5::Shader* vertexShader;
		Graphics5::Shader* fragmentShader;
		Graphics5::Shader* geometryShader;
		Graphics5::Shader* tessellationControlShader;
		Graphics5::Shader* tessellationEvaluationShader;

		Program5Impl();
		virtual ~Program5Impl();

		const char** textures;
		int* textureValues;
		int textureCount;

		VkPipeline pipeline;
		VkPipelineCache pipelineCache;
		VkShaderModule vert_shader_module;
		VkShaderModule frag_shader_module;

		std::map<std::string, u32> vertexLocations;
		std::map<std::string, u32> fragmentLocations;
		std::map<std::string, u32> textureBindings;
		std::map<std::string, u32> vertexOffsets;
		std::map<std::string, u32> fragmentOffsets;

	public:
		VkPipelineLayout pipeline_layout;
		float uniformDataVertex[256];
		float uniformDataFragment[256];
		static Graphics5::Program* current;
	};

	class ConstantLocation5Impl {
	public:
		int vertexOffset;
		int fragmentOffset;
	};
}
