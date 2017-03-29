#pragma once

#include <Kore/Graphics5/Graphics.h>

namespace Kore {
	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
	public:
		Graphics5::IndexBuffer _buffer;
	};
}
