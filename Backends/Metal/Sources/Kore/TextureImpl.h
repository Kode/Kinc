#pragma once

#include <Kore/Graphics/Image.h>
#include <objc/runtime.h>

namespace Kore {
	class Texture;

	class TextureUnitImpl {
	public:
	};
	
	class TextureImpl {
	public:
		~TextureImpl();
		id tex;
	protected:
		void create(int width, int height);
	};
}
