#pragma once

#ifdef KORE_WINDOWS
struct IDirect3DVertexBuffer9;
struct IDirect3DVertexDeclaration9;
#endif

namespace Kore {
	namespace Graphics4 {
		class VertexBuffer;
	}

	class VertexBufferImpl {
	protected:
#ifdef KORE_WINDOWS
		IDirect3DVertexBuffer9* vb;
#endif
		int myCount;
		int myStride;
		int instanceDataStepRate;
		VertexBufferImpl(int count, int instanceDataStepRate);
		void unset();

	public:
		static Graphics4::VertexBuffer* _current;
		int _offset;
	};
}
