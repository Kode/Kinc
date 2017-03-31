#pragma once

namespace Kore {
	namespace Graphics4 {
		class IndexBuffer;
	}

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();

#if defined(KORE_ANDROID) || defined(KORE_PI)
		u16* shortData;
#endif
		int* data;
		int myCount;
		uint bufferId;

	public:
		static Graphics4::IndexBuffer* current;
	};
}
