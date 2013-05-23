#pragma once

struct ID3D11Buffer;

namespace Kore {
	class VertexBuffer;

	class VertexBufferImpl {
	protected:
		VertexBufferImpl(int count);
		ID3D11Buffer* vb;
		int myCount;
		int myStride;
		float* vertices;
	};
}
