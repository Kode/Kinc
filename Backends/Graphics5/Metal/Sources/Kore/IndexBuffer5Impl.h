#pragma once

#include <objc/runtime.h>

namespace Kore {
	namespace Graphics5 {
		class IndexBuffer;
	}

	class IndexBuffer5Impl {
	protected:
	public:
		IndexBuffer5Impl(int count);
		void unset();
		id mtlBuffer;
		int myCount;
		bool gpuMemory;
	public:
		static Graphics5::IndexBuffer* current;
	};
}
