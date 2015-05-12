#pragma once

struct ID3D12Resource;

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
		ID3D12Resource* ib;
		ID3D12Resource* ibUpload;
		int* indices;
		int myCount;
	public:
		static IndexBuffer* _current;
	};
}
