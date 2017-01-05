#pragma once

namespace Kore {
	class RenderTargetImpl {
	public:
		unsigned _framebuffer;
		unsigned _texture;
		unsigned _depthTexture;
		bool _hasDepth;
		// unsigned _depthRenderbuffer;
		int contextId;
		void setupDepthStencil(int depthBufferBits, int stencilBufferBits, int width, int height);
	};
}
