#pragma once

#include <Kore/Graphics/VertexStructure.h>
#include <vulkan/vulkan.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace Kore {
	struct VertexInfo {
		VkPipelineVertexInputStateCreateInfo vi;
		VkVertexInputBindingDescription vi_bindings[1];
		VkVertexInputAttributeDescription vi_attrs[2];
	};

	struct Vertices {
		VkBuffer buf;
		VkDeviceMemory mem;

		VertexInfo info;
	};

	class VertexBuffer;
	
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count, int instanceDataStepRate);
		void unset();
		float* data;
		int myCount;
		int myStride;
		uint bufferId;
//#if defined SYS_ANDROID || defined SYS_HTML5 || defined SYS_TIZEN
		VertexStructure structure;
//#endif

		Vertices vertices;
		VkMemoryAllocateInfo mem_alloc;

		int instanceDataStepRate;
		int setVertexAttributes(int offset);
	public:
		static VertexBuffer* current;
	};
}
