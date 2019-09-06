#include "pch.h"

#include <kinc/graphics4/rendertarget.h>
#include <kinc/log.h>
#include <kinc/error.h>

#include "Direct3D11.h"

#include <Kore/SystemMicrosoft.h>

static DXGI_FORMAT convertFormat(kinc_g4_render_target_format_t format) {
	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(kinc_g4_render_target_format_t format) {
	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return 16;
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return 8;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return 4;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return 2;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
		return 1;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		return 4;
	}
}

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, int depthBufferBits, bool antialiasing, kinc_g4_render_target_format_t format, int stencilBufferBits,
                                      int contextId) {
	renderTarget->isCubeMap = false;
	renderTarget->isDepthAttachment = false;
	renderTarget->texWidth = renderTarget->width = width;
	renderTarget->texHeight = renderTarget->height = height;
	renderTarget->contextId = contextId;
	renderTarget->impl.format = format;
	renderTarget->impl.textureStaging = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = convertFormat(format);
	if (format == KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH) {
		renderTarget->isDepthAttachment = true;
		depthBufferBits = 16;
		stencilBufferBits = 0;
	}

	if (antialiasing) {
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	}
	else {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	renderTarget->impl.textureRender = nullptr;
	renderTarget->impl.textureSample = nullptr;
	renderTarget->impl.renderTargetSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTarget->impl.renderTargetViewRender[i] = nullptr;
		renderTarget->impl.renderTargetViewSample[i] = nullptr;
	}
	if (!renderTarget->isDepthAttachment) {
		kinc_microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &renderTarget->impl.textureRender));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = antialiasing ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		kinc_microsoft_affirm(
		    device->CreateRenderTargetView(renderTarget->impl.textureRender, &renderTargetViewDesc, &renderTarget->impl.renderTargetViewRender[0]));

		if (antialiasing) {
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			kinc_microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &renderTarget->impl.textureSample));

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			kinc_microsoft_affirm(
			    device->CreateRenderTargetView(renderTarget->impl.textureSample, &renderTargetViewDesc, &renderTarget->impl.renderTargetViewSample[0]));
		}
		else {
			renderTarget->impl.textureSample = renderTarget->impl.textureRender;
			renderTarget->impl.renderTargetViewSample[0] = renderTarget->impl.renderTargetViewRender[0];
		}
	}

	renderTarget->impl.depthStencil = nullptr;
	renderTarget->impl.depthStencilSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTarget->impl.depthStencilView[i] = nullptr;
	}
	
	DXGI_FORMAT depthFormat;
	DXGI_FORMAT depthViewFormat;
	DXGI_FORMAT depthResourceFormat;
	if (depthBufferBits == 16 && stencilBufferBits == 0) {
	 	depthFormat = DXGI_FORMAT_R16_TYPELESS;
	 	depthViewFormat = DXGI_FORMAT_D16_UNORM;
	 	depthResourceFormat = DXGI_FORMAT_R16_UNORM;
	}
	else {
		depthFormat = DXGI_FORMAT_R24G8_TYPELESS;
		depthViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthResourceFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	if (depthBufferBits > 0) {
		CD3D11_TEXTURE2D_DESC depthStencilDesc(depthFormat, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
		if (antialiasing) {
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}
		else {
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		kinc_microsoft_affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &renderTarget->impl.depthStencil));
		kinc_microsoft_affirm(device->CreateDepthStencilView(
		    renderTarget->impl.depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(antialiasing ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D, depthViewFormat), &renderTarget->impl.depthStencilView[0]));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!renderTarget->isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = antialiasing ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		kinc_microsoft_affirm(device->CreateShaderResourceView(renderTarget->impl.textureSample, &shaderResourceViewDesc, &renderTarget->impl.renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = depthResourceFormat;
		shaderResourceViewDesc.ViewDimension = antialiasing ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		kinc_microsoft_affirm(device->CreateShaderResourceView(renderTarget->impl.depthStencil, &shaderResourceViewDesc, &renderTarget->impl.depthStencilSRV));
	}

	if (renderTarget->impl.renderTargetViewRender[0] != nullptr) {
		FLOAT colors[4] = {0, 0, 0, 0};
		context->ClearRenderTargetView(renderTarget->impl.renderTargetViewRender[0], colors);
	}
}

void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g4_render_target_format_t format, int stencilBufferBits, int contextId) {
	renderTarget->width = cubeMapSize;
	renderTarget->height = cubeMapSize;
	renderTarget->isCubeMap = true;
	renderTarget->isDepthAttachment = false;

	renderTarget->texWidth = renderTarget->width;
	renderTarget->texHeight = renderTarget->height;
	renderTarget->contextId = contextId;
	renderTarget->impl.format = format;
	renderTarget->impl.textureStaging = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = renderTarget->width;
	desc.Height = renderTarget->height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = convertFormat(format);
	if (format == KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH) {
		renderTarget->isDepthAttachment = true;
		depthBufferBits = 16;
		stencilBufferBits = 0;
	}

	if (antialiasing) {
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	}
	else {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	renderTarget->impl.textureRender = nullptr;
	renderTarget->impl.textureSample = nullptr;
	renderTarget->impl.renderTargetSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTarget->impl.renderTargetViewRender[i] = nullptr;
		renderTarget->impl.renderTargetViewSample[i] = nullptr;
	}
	if (!renderTarget->isDepthAttachment) {
		kinc_microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &renderTarget->impl.textureRender));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;

		for (int i = 0; i < 6; i++) {
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			kinc_microsoft_affirm(
			    device->CreateRenderTargetView(renderTarget->impl.textureRender, &renderTargetViewDesc, &renderTarget->impl.renderTargetViewRender[i]));
		}

		if (antialiasing) {
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			kinc_microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &renderTarget->impl.textureSample));

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			renderTargetViewDesc.Texture2DArray.ArraySize = 1;
			for (int i = 0; i < 6; i++) {
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
				kinc_microsoft_affirm(
				    device->CreateRenderTargetView(renderTarget->impl.textureSample, &renderTargetViewDesc, &renderTarget->impl.renderTargetViewSample[i]));
			}
		}
		else {
			renderTarget->impl.textureSample = renderTarget->impl.textureRender;
			for (int i = 0; i < 6; i++) {
				renderTarget->impl.renderTargetViewSample[i] = renderTarget->impl.renderTargetViewRender[i];
			}
		}
	}

	renderTarget->impl.depthStencil = nullptr;
	renderTarget->impl.depthStencilSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTarget->impl.depthStencilView[i] = nullptr;
	}
	
	DXGI_FORMAT depthFormat;
	DXGI_FORMAT depthViewFormat;
	DXGI_FORMAT depthResourceFormat;
	if (depthBufferBits == 16 && stencilBufferBits == 0) {
	 	depthFormat = DXGI_FORMAT_R16_TYPELESS;
	 	depthViewFormat = DXGI_FORMAT_D16_UNORM;
	 	depthResourceFormat = DXGI_FORMAT_R16_UNORM;
	}
	else {
		depthFormat = DXGI_FORMAT_R24G8_TYPELESS;
		depthViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthResourceFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	if (depthBufferBits > 0) {
		CD3D11_TEXTURE2D_DESC depthStencilDesc(depthFormat, renderTarget->width, renderTarget->height, 1, 1,
		                                       D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
		depthStencilDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		depthStencilDesc.ArraySize = 6;
		if (antialiasing) {
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}
		else {
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		kinc_microsoft_affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &renderTarget->impl.depthStencil));

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = depthViewFormat;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilViewDesc.Texture2DArray.MipSlice = 0;
		depthStencilViewDesc.Texture2DArray.ArraySize = 1;
		depthStencilViewDesc.Flags = 0;
		for (int i = 0; i < 6; i++) {
			depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
			kinc_microsoft_affirm(
			    device->CreateDepthStencilView(renderTarget->impl.depthStencil, &depthStencilViewDesc, &renderTarget->impl.depthStencilView[i]));
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!renderTarget->isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		kinc_microsoft_affirm(device->CreateShaderResourceView(renderTarget->impl.textureSample, &shaderResourceViewDesc, &renderTarget->impl.renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = depthResourceFormat;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		kinc_microsoft_affirm(device->CreateShaderResourceView(renderTarget->impl.depthStencil, &shaderResourceViewDesc, &renderTarget->impl.depthStencilSRV));
	}

	if (!renderTarget->isDepthAttachment) {
		FLOAT colors[4] = {0, 0, 0, 0};
		for (int i = 0; i < 6; i++) {
			context->ClearRenderTargetView(renderTarget->impl.renderTargetViewRender[i], colors);
		}
	}
}

void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget) {
	for (int i = 0; i < 6; i++) {
		if (renderTarget->impl.renderTargetViewRender[i] != nullptr) renderTarget->impl.renderTargetViewRender[i]->Release();
		if (renderTarget->impl.renderTargetViewSample[i] != nullptr && renderTarget->impl.renderTargetViewSample[i] != renderTarget->impl.renderTargetViewRender[i])
			renderTarget->impl.renderTargetViewSample[i]->Release();
		if (renderTarget->impl.depthStencilView[i] != nullptr) renderTarget->impl.depthStencilView[i]->Release();
	}
	if (renderTarget->impl.renderTargetSRV != nullptr) renderTarget->impl.renderTargetSRV->Release();
	if (renderTarget->impl.depthStencilSRV != nullptr) renderTarget->impl.depthStencilSRV->Release();
	if (renderTarget->impl.depthStencil != nullptr) renderTarget->impl.depthStencil->Release();
	if (renderTarget->impl.textureRender != nullptr) renderTarget->impl.textureRender->Release();
	if (renderTarget->impl.textureStaging != nullptr) renderTarget->impl.textureStaging->Release();
}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	if (unit.impl.unit < 0) return;
	if (renderTarget->impl.textureSample != renderTarget->impl.textureRender) {
		context->ResolveSubresource(renderTarget->impl.textureSample, 0, renderTarget->impl.textureRender, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	if (unit.impl.vertex) {
		context->VSSetShaderResources(unit.impl.unit, 1, renderTarget->isDepthAttachment ? &renderTarget->impl.depthStencilSRV : &renderTarget->impl.renderTargetSRV);
	}
	else {
		context->PSSetShaderResources(unit.impl.unit, 1, renderTarget->isDepthAttachment ? &renderTarget->impl.depthStencilSRV : &renderTarget->impl.renderTargetSRV);
	}
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	if (unit.impl.unit < 0) return;
	if (unit.impl.vertex) {
		context->VSSetShaderResources(unit.impl.unit, 1, &renderTarget->impl.depthStencilSRV);
	}
	else {
		context->PSSetShaderResources(unit.impl.unit, 1, &renderTarget->impl.depthStencilSRV);
	}
}

void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source) {
	renderTarget->impl.depthStencil = source->impl.depthStencil;
	for (int i = 0; i < 6; i++) {
		renderTarget->impl.depthStencilView[i] = source->impl.depthStencilView[i];
	}
	renderTarget->impl.depthStencilSRV = source->impl.depthStencilSRV;
}

void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data) {
	if (renderTarget->impl.textureStaging == nullptr) {
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = renderTarget->texWidth;
		desc.Height = renderTarget->texHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = convertFormat((kinc_g4_render_target_format_t)renderTarget->impl.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;
		kinc_microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &renderTarget->impl.textureStaging));
	}

	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.right = renderTarget->texWidth;
	sourceRegion.top = 0;
	sourceRegion.bottom = renderTarget->texHeight;
	sourceRegion.front = 0;
	sourceRegion.back = 1;
	context->CopySubresourceRegion(renderTarget->impl.textureStaging, 0, 0, 0, 0, renderTarget->impl.textureRender, 0, &sourceRegion);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(renderTarget->impl.textureStaging, 0, D3D11_MAP_READ, 0, &mappedResource);
	memcpy(data, mappedResource.pData, renderTarget->texWidth * renderTarget->texHeight * formatByteSize((kinc_g4_render_target_format_t)renderTarget->impl.format));
	context->Unmap(renderTarget->impl.textureStaging, 0);
}

void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels) {}
