#pragma once

//#include <Kore/Graphics/Graphics.h>

#ifdef SYS_WINDOWS
struct IDirect3DIndexBuffer9;
#endif
#ifdef SYS_XBOX360
struct D3DIndexBuffer;
#endif

namespace Kore {
	namespace Graphics4 {
		class IndexBuffer;
	}

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
#ifdef SYS_WINDOWS
		IDirect3DIndexBuffer9* ib;
#endif
#ifdef SYS_XBOX360
		D3DIndexBuffer* ib;
#endif
		int myCount;

	public:
		static Graphics4::IndexBuffer* _current;
	};
}
