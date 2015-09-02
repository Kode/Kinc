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
		ID3D12Resource* indexBuffer_;
		D3D12IindexBufferView indexBufferView_;
		ID3D12Resource* uploadBuffer_;
		int myCount;
		static IndexBufferImpl* _current;
	};
}
