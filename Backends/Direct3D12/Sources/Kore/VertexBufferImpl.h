#pragma once

struct ID3D12Resource;

struct D3D12VertexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	unsigned int StrideInBytes;
};

namespace Kore {
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
	public:
		ID3D12Resource* vb;
		D3D12VertexBufferView view;
		int myCount;
		int myStride;
		//float* vertices;
		static VertexBufferImpl* _current;
	};
}
