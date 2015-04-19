#pragma once

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();
	
		int* data;
		int myCount;
	public:
		static IndexBuffer* current;
	};
}
