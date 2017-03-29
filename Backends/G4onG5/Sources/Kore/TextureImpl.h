#pragma once

#include <Kore/Graphics5/Graphics.h>

namespace Kore {
	class TextureUnitImpl {
	public:
		Kore::Graphics5::TextureUnit _unit;
	};

	class TextureImpl {
	public:
		~TextureImpl();
		void unmipmap();
		void unset();

		Kore::Graphics5::Texture* _texture;
	};
}
