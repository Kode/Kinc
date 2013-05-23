#pragma once

struct ID3D11Buffer;

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
		ID3D11Buffer* ib;
		int* indices;
		int myCount;
	public:
		static IndexBuffer* _current;
	};
}
