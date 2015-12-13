#pragma once

#include <Kore/Graphics/VertexStructure.h>

namespace Kore {
	class VertexBuffer;
	
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count, int instanceDataStepRate);
		void unset();
		float* data;
		int myCount;
		int myStride;
		uint bufferId;
//#if defined SYS_ANDROID || defined SYS_HTML5 || defined SYS_TIZEN
		VertexStructure structure;
//#endif
		int instanceDataStepRate;
		int setVertexAttributes(int offset);
	public:
		static VertexBuffer* current;
	};
}
