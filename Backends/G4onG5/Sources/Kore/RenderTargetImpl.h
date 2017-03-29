#pragma once

#include <Kore/Graphics5/Graphics.h>

namespace Kore {
	class RenderTargetImpl {
	public:
		RenderTargetImpl(int width, int height, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId);
		RenderTargetImpl(int cubeMapSize, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId);
		Graphics5::RenderTarget _renderTarget;
	};
}
