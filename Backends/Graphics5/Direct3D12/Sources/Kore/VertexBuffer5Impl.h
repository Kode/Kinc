#pragma once

struct ID3D12Resource;

struct D3D12VertexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	unsigned int StrideInBytes;
};

namespace Kore {
	class VertexBuffer5Impl {
	protected:
		VertexBuffer5Impl(int count);

	public:
		// ID3D12Resource* vertexBuffer;
		ID3D12Resource* uploadBuffer;
		D3D12VertexBufferView view;

		int myCount;
		int myStride;
		int lastStart;
		int lastCount;
		// float* vertices;
		static VertexBuffer5Impl* _current;
	};
}
