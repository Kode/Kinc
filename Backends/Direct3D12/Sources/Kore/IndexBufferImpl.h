#pragma once

struct ID3D12Resource;

namespace Kore {
	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
	public:
		ID3D12Resource* ib;
		ID3D12Resource* ibUpload;
		int* indices;
		int myCount;
		static IndexBufferImpl* _current;
	};
}
