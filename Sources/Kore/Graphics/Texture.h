#pragma once

#include <Kore/TextureImpl.h>
#include <Kore/Graphics/Image.h>

namespace Kore {
	class TextureUnit : public TextureUnitImpl {

	};

	class Texture : public Image, public TextureImpl {
	public:
		Texture(int width, int height, Format format);
		Texture(const char* filename);
		void set(TextureUnit unit);
		int texWidth;
		int texHeight;
	};
}
