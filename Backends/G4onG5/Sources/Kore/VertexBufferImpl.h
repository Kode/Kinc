#pragma once

#include <Kore/Graphics5/Graphics.h>

namespace Kore {
	class VertexStructure;

	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count, const VertexStructure& structure, int instanceDataStepRate);

	public:
		Graphics5::VertexBuffer _buffer;
	};
}
