#pragma once

#include <objc/runtime.h>

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();
		int offset();
		id mtlBuffer;
		int myCount;
		int index;
	public:
		static IndexBuffer* current;
	};
}
