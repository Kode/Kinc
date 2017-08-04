#pragma once

#include <Kore/Graphics5/VertexStructure.h>
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
		//#if defined KORE_ANDROID || defined KORE_HTML5 || defined KORE_TIZEN
		Graphics5::VertexStructure structure;
		//#endif

		VkMemoryAllocateInfo mem_alloc;

		int instanceDataStepRate;
		int setVertexAttributes(int offset);

	public:
		int index;
		Vertices vertices;
		static Graphics5::VertexBuffer* current;
	};
}
