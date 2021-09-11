#pragma once

#include <Runtime/RHI/Public/RHI.h>
#include <Runtime/RHI/Public/RHIResources.h>

namespace Kore {
	class VertexBuffer;

	class VertexBufferImpl {
	protected:
		int myCount;
		int myStride;
		int instanceDataStepRate;
		VertexBufferImpl(int count, int instanceDataStepRate);
		void unset();
		FVertexBufferRHIRef vertexBuffer;

	public:
		static VertexBuffer *_current;
		int _offset;
	};
}
