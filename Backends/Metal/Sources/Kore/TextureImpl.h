#pragma once

#include <Kore/Graphics/Image.h>

namespace Kore {
	class Texture;

	class TextureUnitImpl {
	public:
	};
	
	class TextureImpl {
	public:
		~TextureImpl();
		void* tex;
	protected:
		void create(int width, int height);
	};
}
