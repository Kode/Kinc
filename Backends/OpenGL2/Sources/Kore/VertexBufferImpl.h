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
		uint arrayId;
		uint bufferId;
#if defined SYS_ANDROID || defined SYS_HTML5
		VertexStructure structure;
#endif
	public:
		static VertexBuffer* current;
	};
}

