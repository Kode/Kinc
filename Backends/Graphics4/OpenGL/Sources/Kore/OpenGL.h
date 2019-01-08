#pragma once

#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Matrix.h>

namespace Kore {
	namespace OpenGL {
		int textureAddressingU(Graphics4::TextureUnit unit);
		int textureAddressingV(Graphics4::TextureUnit unit);
		int stencilFunc(Graphics4::ZCompareMode mode);
	}
}
