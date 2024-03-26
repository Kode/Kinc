#pragma once

namespace Kore {
	namespace Graphics3 {
		class IndexBuffer;
	}

	class IndexBufferImpl {
	protected:
	public:
		IndexBufferImpl(int count);
		void unset();

#if defined(KINC_ANDROID) || defined(KINC_PI)
		u16 *shortData;
#endif
		int *data;
		int myCount;
		uint bufferId;

	public:
		static Graphics3::IndexBuffer *current;
	};
}
