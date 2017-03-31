#pragma once

//#include <Kore/Graphics/Graphics.h>

#ifdef KORE_WINDOWS
struct IDirect3DIndexBuffer9;
#endif

namespace Kore {
	namespace Graphics4 {
		class IndexBuffer;
	}

	class IndexBufferImpl {
	protected:
		IndexBufferImpl(int count);
#ifdef KORE_WINDOWS
		IDirect3DIndexBuffer9* ib;
#endif
		int myCount;

	public:
		static Graphics4::IndexBuffer* _current;
	};
}
