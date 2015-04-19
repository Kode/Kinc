#pragma once

#include <Kore/Graphics/VertexStructure.h>

namespace Kore {
	class VertexBuffer;
	
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
		void unset();
		float* data;
		int myCount;
		int myStride;
	public:
		static VertexBuffer* current;
	};
}

