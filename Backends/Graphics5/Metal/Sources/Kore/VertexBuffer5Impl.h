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
		int offset();
		int myCount;
		int myStride;
		int index;
		id mtlBuffer;

	public:
		static Graphics5::VertexBuffer* current;
	};
}
