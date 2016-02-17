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

		u8 pixfmt;

		~TextureImpl();
		u8* conversionBuffer; // Fuer wenn Textur aus Image erstellt wird
	};
}
