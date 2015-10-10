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
		uint arrayId;
		uint bufferId;
//#if defined SYS_ANDROID || defined SYS_HTML5 || defined SYS_TIZEN
		VertexStructure structure;
//#endif
		int instanceDataStepRate;
	public:
		static VertexBuffer* current;
	};
}
