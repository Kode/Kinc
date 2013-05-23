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
		uint bufferId;
	public:
		static IndexBuffer* current;
	};
}
