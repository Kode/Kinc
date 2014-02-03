#pragma once

struct IDirect3DSurface9;
struct IDirect3DTexture9;

namespace Kore {
	class RenderTargetImpl {
	public:
		unsigned _framebuffer;
		unsigned _texture;
	};
}
