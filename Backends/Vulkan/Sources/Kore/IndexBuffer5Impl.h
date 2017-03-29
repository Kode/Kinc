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
	namespace Graphics5 {
		class IndexBuffer;
	}

	class IndexBuffer5Impl {
	protected:
	public:
		IndexBuffer5Impl(int count);
		void unset();

		int* data;
		int myCount;
		uint bufferId;

		VkBuffer buf;
		VkDeviceMemory mem;
		VkMemoryAllocateInfo mem_alloc;

	public:
		static Graphics5::IndexBuffer* current;
	};
}
