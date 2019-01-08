#include "pch.h"

#include "Direct3D11.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/SystemMicrosoft.h>

using namespace Kore;

Graphics4::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits,
                                      int contextId)
    : isCubeMap(false), isDepthAttachment(false) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;
	this->contextId = contextId;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;

	switch (format) {
	case Target128BitFloat:
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case Target64BitFloat:
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case Target32BitRedFloat:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case Target16BitRedFloat:
		desc.Format = DXGI_FORMAT_R16_FLOAT;
		break;
	case Target8BitRed:
		desc.Format = DXGI_FORMAT_R8_UNORM;
		break;
	case Target16BitDepth:
		isDepthAttachment = true;
		depthBufferBits = 16;
		stencilBufferBits = 0;
		break;
	case Target32Bit:
	default:
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	textureRender = nullptr;
	textureSample = nullptr;
	renderTargetSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTargetViewRender[i] = nullptr;
		renderTargetViewSample[i] = nullptr;
	}
	if (!isDepthAttachment) {
		Kore_Microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &textureRender));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = antialiasing ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		Kore_Microsoft_affirm(device->CreateRenderTargetView(textureRender, &renderTargetViewDesc, &renderTargetViewRender[0]));

		if (antialiasing) {
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			Kore_Microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &textureSample));

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			Kore_Microsoft_affirm(device->CreateRenderTargetView(textureSample, &renderTargetViewDesc, &renderTargetViewSample[0]));
		}
		else {
			textureSample = textureRender;
			renderTargetViewSample[0] = renderTargetViewRender[0];
		}
	}

	depthStencil = nullptr;
	depthStencilSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		depthStencilView[i] = nullptr;
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
		Kore_Microsoft_affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));
		Kore_Microsoft_affirm(device->CreateDepthStencilView(
		    depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, depthViewFormat), &depthStencilView[0]));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = antialiasing ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		Kore_Microsoft_affirm(device->CreateShaderResourceView(textureSample, &shaderResourceViewDesc, &renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = depthResourceFormat;
		shaderResourceViewDesc.ViewDimension = antialiasing ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		Kore_Microsoft_affirm(device->CreateShaderResourceView(depthStencil, &shaderResourceViewDesc, &depthStencilSRV));
	}

	if (renderTargetViewRender[0] != nullptr) {
		FLOAT colors[4] = {0, 0, 0, 0};
		context->ClearRenderTargetView(renderTargetViewRender[0], colors);
	}
}

Graphics4::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
    : width(cubeMapSize), height(cubeMapSize), isCubeMap(true), isDepthAttachment(false) {

	this->texWidth = this->width = width;
	this->texHeight = this->height = height;
	this->contextId = contextId;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;

	switch (format) {
	case Target128BitFloat:
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case Target64BitFloat:
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case Target32BitRedFloat:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case Target16BitRedFloat:
		desc.Format = DXGI_FORMAT_R16_FLOAT;
		break;
	case Target8BitRed:
		desc.Format = DXGI_FORMAT_R8_UNORM;
		break;
	case Target16BitDepth:
		isDepthAttachment = true;
		depthBufferBits = 16;
		stencilBufferBits = 0;
		break;
	case Target32Bit:
	default:
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	textureRender = nullptr;
	textureSample = nullptr;
	renderTargetSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTargetViewRender[i] = nullptr;
		renderTargetViewSample[i] = nullptr;
	}
	if (!isDepthAttachment) {
		Kore_Microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &textureRender));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;

		for (int i = 0; i < 6; i++) {
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			Kore_Microsoft_affirm(device->CreateRenderTargetView(textureRender, &renderTargetViewDesc, &renderTargetViewRender[i]));
		}

		if (antialiasing) {
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			Kore_Microsoft_affirm(device->CreateTexture2D(&desc, nullptr, &textureSample));

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			renderTargetViewDesc.Texture2DArray.ArraySize = 1;
			for (int i = 0; i < 6; i++) {
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
				Kore_Microsoft_affirm(device->CreateRenderTargetView(textureSample, &renderTargetViewDesc, &renderTargetViewSample[i]));
			}
		}
		else {
			textureSample = textureRender;
			for (int i = 0; i < 6; i++) {
				renderTargetViewSample[i] = renderTargetViewRender[i];
			}
		}
	}

	depthStencil = nullptr;
	depthStencilSRV = nullptr;
	for (int i = 0; i < 6; i++) {
		depthStencilView[i] = nullptr;
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
		Kore_Microsoft_affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = depthViewFormat;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilViewDesc.Texture2DArray.MipSlice = 0;
		depthStencilViewDesc.Texture2DArray.ArraySize = 1;
		depthStencilViewDesc.Flags = 0;
		for (int i = 0; i < 6; i++) {
			depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
			Kore_Microsoft_affirm(device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &depthStencilView[i]));
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		Kore_Microsoft_affirm(device->CreateShaderResourceView(textureSample, &shaderResourceViewDesc, &renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = depthResourceFormat;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		Kore_Microsoft_affirm(device->CreateShaderResourceView(depthStencil, &shaderResourceViewDesc, &depthStencilSRV));
	}

	if (!isDepthAttachment) {
		FLOAT colors[4] = {0, 0, 0, 0};
		for (int i = 0; i < 6; i++) {
			context->ClearRenderTargetView(renderTargetViewRender[i], colors);
		}
	}
}

Graphics4::RenderTarget::~RenderTarget() {
	for (int i = 0; i < 6; i++) {
		if (renderTargetViewRender[i] != nullptr) renderTargetViewRender[i]->Release();
		if (renderTargetViewSample[i] != nullptr && renderTargetViewSample[i] != renderTargetViewRender[i]) renderTargetViewSample[i]->Release();
		if (depthStencilView[i] != nullptr) depthStencilView[i]->Release();
	}
	if (renderTargetSRV != nullptr) renderTargetSRV->Release();
	if (depthStencilSRV != nullptr) depthStencilSRV->Release();
	if (textureRender != nullptr) textureRender->Release();
}

void Graphics4::RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	if (textureSample != textureRender) {
		context->ResolveSubresource(textureSample, 0, textureRender, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	context->PSSetShaderResources(unit.unit, 1, isDepthAttachment ? &depthStencilSRV : &renderTargetSRV);
}

void Graphics4::RenderTarget::useDepthAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &depthStencilSRV);
}

void Graphics4::RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	depthStencil = source->depthStencil;
	for (int i = 0; i < 6; i++) {
		depthStencilView[i] = source->depthStencilView[i];
	}
	depthStencilSRV = source->depthStencilSRV;
}

void Graphics4::RenderTarget::getPixels(u8* data) {}

void Graphics4::RenderTarget::generateMipmaps(int levels) {}
