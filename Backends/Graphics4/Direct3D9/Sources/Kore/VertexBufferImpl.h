#pragma once

#ifdef SYS_WINDOWS
struct IDirect3DVertexBuffer9;
struct IDirect3DVertexDeclaration9;
#endif

#ifdef SYS_XBOX360
struct D3DVertexBuffer;
#endif

namespace Kore {
	namespace Graphics4 {
		class VertexBuffer;
	}

	class VertexBufferImpl {
	protected:
#ifdef SYS_WINDOWS
		IDirect3DVertexBuffer9* vb;
#endif
#ifdef SYS_XBOX360
		D3DVertexBuffer* vb;
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
