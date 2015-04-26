#pragma once

#include <Kore/Graphics/VertexStructure.h>
#include <objc/runtime.h>

namespace Kore {
	class VertexBuffer;
	
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
		void unset();
		int myCount;
		int myStride;
		id mtlBuffer;
	public:
		static VertexBuffer* current;
	};
}

