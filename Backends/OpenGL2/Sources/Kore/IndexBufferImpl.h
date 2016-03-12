#pragma once

namespace Kore {
	class IndexBuffer;

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();
	
#if defined(SYS_ANDROID) || defined(SYS_PI)
		u16* shortData;
#endif
		int* data;
		int myCount;
		uint bufferId;
	public:
		static IndexBuffer* current;
	};
}
