#pragma once

#include <Kore/Graphics1/Image.h>

#include <objc/runtime.h>

namespace Kore {
	namespace Graphics5 {
		class Texture;
	}

	class TextureUnit5Impl {
	public:
		int index;
	};

	class Texture5Impl {
	public:
		Texture5Impl();
		~Texture5Impl();
		id _tex;
		id _sampler;
	protected:
		void create(int width, int height, int format, bool writable);
	};
}
