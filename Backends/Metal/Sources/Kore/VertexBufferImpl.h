#pragma once

#include <Kore/Graphics/VertexStructure.h>

namespace Kore {
	class VertexBuffer;
	
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
		void unset();
		int myCount;
		int myStride;
		void* mtlBuffer;
	public:
		static VertexBuffer* current;
	};
}

