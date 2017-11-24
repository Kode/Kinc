#include "pch.h"

#include "Direct3D9.h"
#include "RenderTargetImpl.h"

#include <Kore/Log.h>
#include <Kore/WinError.h>

using namespace Kore;

Graphics4::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, Graphics4::RenderTargetFormat format, int stencilBufferBits, int contextId)
	: width(width), height(height), texWidth(width), texHeight(height), isCubeMap(false), isDepthAttachment(false) {
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

	colorSurface = nullptr;
	depthSurface = nullptr;
	colorTexture = nullptr;
	depthTexture = nullptr;

	if (antialiasing) {
		affirm(device->CreateRenderTarget(width, height, d3dformat, D3DMULTISAMPLE_8_SAMPLES, 0, FALSE, &colorSurface, nullptr));
		affirm(device->CreateDepthStencilSurface(width, height, D3DFMT_D24S8, D3DMULTISAMPLE_8_SAMPLES, 0, TRUE, &depthSurface, nullptr));
		affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &colorTexture, nullptr));
		// affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &depthTexture, nullptr));
		depthTexture = nullptr;
	}
	else {
		if (format == Target16BitDepth) {
			affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)MAKEFOURCC('N', 'U', 'L', 'L'), D3DPOOL_DEFAULT, &colorTexture, nullptr));
			affirm(device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), D3DPOOL_DEFAULT, &depthTexture, nullptr));
			isDepthAttachment = true;
		}
		else {
			affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &colorTexture, nullptr));
			affirm(device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, D3DPOOL_DEFAULT, &depthTexture, nullptr));
		}
		affirm(colorTexture->GetSurfaceLevel(0, &colorSurface));
		affirm(depthTexture->GetSurfaceLevel(0, &depthSurface));
	}
}

Graphics4::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
	: isCubeMap(true), isDepthAttachment(false) {
	
}

Graphics4::RenderTarget::~RenderTarget() {
	if (colorSurface != nullptr) colorSurface->Release();
	if (depthSurface != nullptr) depthSurface->Release();
	if (colorTexture != nullptr) colorTexture->Release();
	if (depthTexture != nullptr) depthTexture->Release();
}

void Graphics4::RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (antialiasing) {
		IDirect3DSurface9* surface;
		colorTexture->GetSurfaceLevel(0, &surface);
		affirm(device->StretchRect(colorSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
		surface->Release();
	}
	device->SetTexture(unit.unit, isDepthAttachment ? depthTexture : colorTexture);
}

void Graphics4::RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	depthTexture = source->depthTexture;
	depthSurface = source->depthSurface;
}

void Graphics4::RenderTarget::useDepthAsTexture(TextureUnit unit) {
    if (antialiasing) {
        IDirect3DSurface9* surface;
        depthTexture->GetSurfaceLevel(0, &surface);
        affirm(device->StretchRect(depthSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
        surface->Release();
    }
    device->SetTexture(unit.unit, depthTexture);
}

void Graphics4::RenderTarget::getPixels(u8* data) {}

void Graphics4::RenderTarget::generateMipmaps(int levels) {}
