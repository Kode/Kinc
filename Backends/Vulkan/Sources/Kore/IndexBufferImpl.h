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

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();
	
		int* data;
		int myCount;
		uint bufferId;

		VkBuffer buf;
		VkDeviceMemory mem;
		VkMemoryAllocateInfo mem_alloc;
	public:
		static IndexBuffer* current;
	};
}
