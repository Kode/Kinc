#pragma once

#include <Kore/Graphics5/VertexStructure.h>
#include <objc/runtime.h>

namespace Kore {
	namespace Graphics5 {
		class VertexBuffer;
	}

	class VertexBuffer5Impl {
	protected:
		VertexBuffer5Impl(int count);
		void unset();
		int myCount;
		int myStride;
		id mtlBuffer;
		bool gpuMemory;
		int lastStart;
		int lastCount;
	public:
		~VertexBuffer5Impl();
		static Graphics5::VertexBuffer* current;
	};
}
