#include "pch.h"

#include <kinc/graphics4/rendertarget.h>
#include <kinc/log.h>

#include <Kore/SystemMicrosoft.h>

#include "Direct3D9.h"

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {
	renderTarget->width = width;
	renderTarget->height = height;
	renderTarget->texWidth = width;
	renderTarget->texHeight = height;
	renderTarget->isCubeMap = false;
	renderTarget->isDepthAttachment = false;

	renderTarget->impl.antialiasing = antialiasing;
	renderTarget->contextId = contextId;

	D3DFORMAT d3dformat;
	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		d3dformat = D3DFMT_A16B16G16R16F;
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		d3dformat = D3DFMT_R32F;
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		d3dformat = D3DFMT_A8R8G8B8;
	}

#if defined(_DEBUG)
	kinc_log(KINC_LOG_LEVEL_INFO, "depthBufferBits not implemented yet, using defaults (D3DFMT_D24S8)");
	kinc_log(KINC_LOG_LEVEL_INFO, "stencilBufferBits not implemented yet, using defaults (D3DFMT_D24S8)");
#endif

	renderTarget->impl.colorSurface = nullptr;
	renderTarget->impl.depthSurface = nullptr;
	renderTarget->impl.colorTexture = nullptr;
	renderTarget->impl.depthTexture = nullptr;

	if (antialiasing) {
		kinc_microsoft_affirm(
		    device->CreateRenderTarget(width, height, d3dformat, D3DMULTISAMPLE_8_SAMPLES, 0, FALSE, &renderTarget->impl.colorSurface, nullptr));
		kinc_microsoft_affirm(
		    device->CreateDepthStencilSurface(width, height, D3DFMT_D24S8, D3DMULTISAMPLE_8_SAMPLES, 0, TRUE, &renderTarget->impl.depthSurface, nullptr));
		kinc_microsoft_affirm(
		    device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &renderTarget->impl.colorTexture, nullptr));
		// Microsoft::affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &depthTexture, nullptr));
		renderTarget->impl.depthTexture = nullptr;
	}
	else {
		if (format == KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH) {
			kinc_microsoft_affirm(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)MAKEFOURCC('N', 'U', 'L', 'L'), D3DPOOL_DEFAULT,
			                                            &renderTarget->impl.colorTexture, nullptr));
			kinc_microsoft_affirm(device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), D3DPOOL_DEFAULT,
			                                            &renderTarget->impl.depthTexture, nullptr));
			renderTarget->isDepthAttachment = true;
		}
		else {
			kinc_microsoft_affirm(
			    device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, d3dformat, D3DPOOL_DEFAULT, &renderTarget->impl.colorTexture, nullptr));
			kinc_microsoft_affirm(
			    device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, D3DPOOL_DEFAULT, &renderTarget->impl.depthTexture, nullptr));
		}
		kinc_microsoft_affirm(renderTarget->impl.colorTexture->GetSurfaceLevel(0, &renderTarget->impl.colorSurface));
		kinc_microsoft_affirm(renderTarget->impl.depthTexture->GetSurfaceLevel(0, &renderTarget->impl.depthSurface));
	}
}

void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {
	renderTarget->isCubeMap = true;
	renderTarget->isDepthAttachment = false;
}

void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget) {
	if (renderTarget->impl.colorSurface != nullptr) {
		renderTarget->impl.colorSurface->Release();
	}
	if (renderTarget->impl.depthSurface != nullptr) {
		renderTarget->impl.depthSurface->Release();
	}
	if (renderTarget->impl.colorTexture != nullptr) {
		renderTarget->impl.colorTexture->Release();
	}
	if (renderTarget->impl.depthTexture != nullptr) {
		renderTarget->impl.depthTexture->Release();
	}
}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	if (renderTarget->impl.antialiasing) {
		IDirect3DSurface9 *surface;
		renderTarget->impl.colorTexture->GetSurfaceLevel(0, &surface);
		kinc_microsoft_affirm(device->StretchRect(renderTarget->impl.colorSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
		surface->Release();
	}
	device->SetTexture(unit.impl.unit, renderTarget->isDepthAttachment ? renderTarget->impl.depthTexture : renderTarget->impl.colorTexture);
}

void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source) {
	renderTarget->impl.depthTexture = source->impl.depthTexture;
	renderTarget->impl.depthSurface = source->impl.depthSurface;
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	if (renderTarget->impl.antialiasing) {
		IDirect3DSurface9 *surface;
		renderTarget->impl.depthTexture->GetSurfaceLevel(0, &surface);
		kinc_microsoft_affirm(device->StretchRect(renderTarget->impl.depthSurface, nullptr, surface, nullptr, D3DTEXF_NONE));
		surface->Release();
	}
	device->SetTexture(unit.impl.unit, renderTarget->impl.depthTexture);
}

void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data) {}

void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels) {}
