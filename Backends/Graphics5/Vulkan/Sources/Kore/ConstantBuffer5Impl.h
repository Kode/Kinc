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
	class ConstantBuffer5Impl {
	public:
		VkBuffer buf;
		VkDescriptorBufferInfo buffer_info;
		VkMemoryAllocateInfo mem_alloc;
		VkDeviceMemory mem;

	protected:
		int lastStart;
		int lastCount;
		int mySize;
		const bool transposeMat3 = true;
		const bool transposeMat4 = true;
	};
}
