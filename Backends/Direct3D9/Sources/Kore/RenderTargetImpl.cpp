#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/WinError.h>
#include <Kore/Log.h>
#include "Direct3D9.h"

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) : width(width), height(height), texWidth(width), texHeight(height) {
	this->antialiasing = antialiasing;
	this->contextId = contextId;
	D3DFORMAT d3dformat;
	switch (format) {
	case Target64BitFloat:
		d3dformat = D3DFMT_A16B16G16R16F;
		break;
	case Target32BitRedFloat:
		d3dformat = D3DFMT_R32F;
		break;
	case Target32Bit:
	default:
		d3dformat = D3DFMT_A8R8G8B8;
	}

#if defined(_DEBUG)
	log(Info, "depthBufferBits not implemented yet, using defaults (D3DFMT_D24S8)");
	log(Info, "stencilBufferBits not implemented yet, using defaults (D3DFMT_D24S8)");
#endif

	if (antialiasing) {
		affirm(device->CreateRenderTarget(width, height, d3dformat, D3DMULTISAMPLE_8_SAMPLES, 0, FALSE, &colorSurface, nullptr));
		affirm(device->CreateDepthStencilSurface(width, height, D3DFMT_D24S8, D3DMULTISAMPLE_8_SAMPLES, 0, TRUE, &depthSurface, nullptr));
		affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &colorTexture, nullptr));
		//affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &depthTexture, nullptr));
		depthTexture = nullptr;
	}
	else {
		affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &colorTexture, nullptr));
		affirm(device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, D3DPOOL_DEFAULT, &depthTexture, nullptr));
		affirm(colorTexture->GetSurfaceLevel(0, &colorSurface));
		affirm(depthTexture->GetSurfaceLevel(0, &depthSurface));
	}
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (antialiasing) {
		IDirect3DSurface9* surface;
		colorTexture->GetSurfaceLevel(0, &surface);
		affirm(device->StretchRect(colorSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
		surface->Release();
	}
	device->SetTexture(unit.unit, colorTexture);
}


void RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	//!TODO Implement
}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {}
/*void RenderTarget::useDepthAsTexture(int texunit) {
	if (antialiasing) {
		IDirect3DSurface9* surface;
		depthTexture->GetSurfaceLevel(0, &surface);
		affirm(device->StretchRect(depthSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
		surface->Release();
	}
	device->SetTexture(texunit, depthTexture);
}*/
