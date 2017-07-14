#pragma once

struct ID3D12Resource;

struct D3D12IindexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

namespace Kore {
	class IndexBuffer5Impl {
	protected:
		IndexBuffer5Impl(int count, bool gpuMemory);

	public:
		ID3D12Resource* indexBuffer;
		D3D12IindexBufferView indexBufferView;
		ID3D12Resource* uploadBuffer;
		int myCount;
		static IndexBuffer5Impl* _current;
		void _upload(ID3D12GraphicsCommandList* commandList);
		bool _gpuMemory;
	};
}
