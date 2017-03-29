#pragma once

#include <Kore/Graphics/VertexStructure.h>
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
	struct Vertices {
		VkBuffer buf;
		VkDeviceMemory mem;
	};

	namespace Graphics5 {
		class VertexBuffer;
	}

	class VertexBuffer5Impl {
	protected:
		VertexBuffer5Impl(int count, int instanceDataStepRate);
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
		int index;

	public:
		static Graphics5::VertexBuffer* current;
	};
}
