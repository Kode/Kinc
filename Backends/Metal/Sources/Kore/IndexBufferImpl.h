#pragma once

#include <objc/runtime.h>

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();
	
		id mtlBuffer;
		int myCount;
	public:
		static IndexBuffer* current;
	};
}
