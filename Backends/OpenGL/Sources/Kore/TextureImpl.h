#pragma once

#include <Kore/Graphics/Image.h>

namespace Kore {
	class Texture;

	class TextureUnitImpl {
	public:
		int unit;
	};
	
	class TextureImpl {
	protected:
		//static TreeMap<Image, Texture*> images;
	public:
		unsigned int texture;
#ifdef SYS_ANDROID
		bool external_oes;
#endif

		u8 pixfmt;

		~TextureImpl();
	};
}
