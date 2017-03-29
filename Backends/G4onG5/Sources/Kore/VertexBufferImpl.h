#pragma once

#include <Kore/Graphics5/Graphics.h>

namespace Kore {
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);

	public:
		Graphics5::VertexBuffer* _buffer;
	};
}
