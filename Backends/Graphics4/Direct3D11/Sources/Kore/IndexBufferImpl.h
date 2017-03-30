#pragma once

struct ID3D11Buffer;

namespace Kore {
	namespace Graphics4 {
		class IndexBuffer;
	}

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
		ID3D11Buffer* ib;
		int* indices;
		int myCount;

	public:
		static Graphics4::IndexBuffer* _current;
	};
}
