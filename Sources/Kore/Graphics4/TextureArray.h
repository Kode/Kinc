#pragma once

#include "Texture.h"

#include <kinc/graphics4/texturearray.h>

namespace Kore {
	namespace Graphics4 {
		class TextureArray {
		public:
			TextureArray(Image** textures, int count);
			~TextureArray();
			kinc_g4_texture_array_t kincArray;
		};
	}
}
