#pragma once

#include <Kore/Graphics1/Image.h>

namespace Kore {
	namespace Graphics4 {
		class Texture;
	}

	class TextureUnitImpl {
	public:
		int unit;
	};

	class TextureImpl {
	protected:
		// static TreeMap<Image, Texture*> images;
	public:
		unsigned int texture;
#ifdef KORE_ANDROID
		bool external_oes;
#endif

		u8 pixfmt;

		~TextureImpl();
	};
}
