#pragma once

namespace Kore {
	namespace Graphics4 {
		class IndexBuffer;
	}

	class IndexBuffer5Impl {
	protected:
		IndexBuffer5Impl(int count, bool gpuMemory);

	public:
		Graphics4::IndexBuffer* buffer;
		int myCount;
		static IndexBuffer5Impl* _current;
	};
}
