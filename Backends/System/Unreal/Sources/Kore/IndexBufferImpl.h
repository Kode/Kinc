#pragma once

#include <Runtime/RHI/Public/RHI.h>
#include <Runtime/RHI/Public/RHIResources.h>

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
		int myCount;

	public:
		static IndexBuffer* _current;
		FIndexBufferRHIRef indexBuffer;
	};
}
