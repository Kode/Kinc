#pragma once

struct ID3D12Resource;

namespace Kore {
	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
	public:
		ID3D12Resource* vb;
		ID3D12Resource* vbUpload;
		int myCount;
		int myStride;
		float* vertices;
		static VertexBufferImpl* _current;
	};
}
