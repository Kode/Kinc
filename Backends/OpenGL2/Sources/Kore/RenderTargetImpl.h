#pragma once

namespace Kore {
	class RenderTargetImpl {
	public:
		unsigned _framebuffer;
		unsigned _texture;
		int contextId;
	};
}
