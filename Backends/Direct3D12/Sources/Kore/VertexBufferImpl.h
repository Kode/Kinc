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
		ID3D12Resource* vertexBuffer_;
		D3D12VertexBufferView vertexBufferView_;
		ID3D12Resource* uploadBuffer_;

		int myCount;
		int myStride;
		//float* vertices;
		static VertexBufferImpl* _current;
	};
}
