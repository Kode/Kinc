#pragma once

namespace Kore {
	namespace Graphics4 {
		class VertexBuffer;
	}

	class VertexBuffer5Impl {
	protected:
		VertexBuffer5Impl(int count);

	public:
		Graphics4::VertexBuffer* buffer;
		int myCount;
		int myStride;
		int myStart;
		// float* vertices;
		static VertexBuffer5Impl* _current;
	};
}
