#pragma once

struct IDirect3DSurface9;
struct IDirect3DTexture9;

namespace Kore {
	class RenderTargetImpl {
	public:
		IDirect3DSurface9* colorSurface;
		IDirect3DSurface9* depthSurface;
		IDirect3DTexture9* colorTexture;
		IDirect3DTexture9* depthTexture;
		bool antialiasing;
	};
}
