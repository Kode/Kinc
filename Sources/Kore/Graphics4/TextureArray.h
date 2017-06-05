#pragma once

#include "Texture.h"

#include <Kore/TextureArrayImpl.h>

namespace Kore {
	namespace Graphics4 {
		class TextureArray : public TextureArrayImpl {
		public:
			TextureArray(Image** textures, int count);
		};
	}
}
