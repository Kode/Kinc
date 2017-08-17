#pragma once

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics1/Image.h>

namespace Kore {
	typedef Graphics1::Image Image;

	class TextureUnitImpl {
	public:
		Kore::Graphics5::TextureUnit _unit;
	};

	class TextureImpl {
	public:
		TextureImpl();
		TextureImpl(int width, int height, Image::Format format, bool readable);
		TextureImpl(int width, int height, int depth, Image::Format format, bool readable);
		~TextureImpl();
		void unmipmap();
		void unset();

		Kore::Graphics5::Texture* _texture;
		bool _uploaded;
	};
}
