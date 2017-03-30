#pragma once

#include <Kore/Graphics4/VertexStructure.h>

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
		Graphics4::VertexStructure structure;
		//#endif
		int instanceDataStepRate;
		int setVertexAttributes(int offset);
#ifndef NDEBUG
		bool initialized;
#endif
	public:
		static VertexBuffer* current;
	};
}
