#pragma once

struct ID3D12Resource;

struct D3D12IindexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

namespace Kore {
	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
	public:
		ID3D12Resource* ib;
		D3D12IindexBufferView view;
		//ID3D12Resource* ibUpload;
		//int* indices;
		int myCount;
		static IndexBufferImpl* _current;
	};
}
