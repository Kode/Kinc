#pragma once

struct ID3D12Resource;

namespace Kore {
	class VertexBuffer;

	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
		ID3D12Resource* vb;
		ID3D12Resource* vbUpload;
		int myCount;
		int myStride;
		float* vertices;
	};
}
