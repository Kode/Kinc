#include "pch.h"

#include "Direct3D11.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>
#include <Kore/WinError.h>

using namespace Kore;

Graphics4::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
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

	texture = nullptr;
	for (int i = 0; i < 6; i++) {
		renderTargetView[i] = nullptr;
	}
	if (!isDepthAttachment) {
		affirm(device->CreateTexture2D(&desc, nullptr, &texture));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		affirm(device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView[0]));
	}

	depthStencil = nullptr;
	depthStencilView = nullptr;
	DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (depthBufferBits == 32 && stencilBufferBits == 8) {
		depthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}
	else if (depthBufferBits == 16 && stencilBufferBits <= 0) {
		depthFormat = DXGI_FORMAT_D16_UNORM;
	}
	else if (stencilBufferBits <= 0) {
		depthFormat = DXGI_FORMAT_D32_FLOAT;
	}

	if (depthBufferBits > 0) {
		CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
		if (antialiasing) {
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}
		else {
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

		affirm(device->CreateDepthStencilView(depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT), &depthStencilView));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		affirm(device->CreateShaderResourceView(texture, &shaderResourceViewDesc, &renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		affirm(device->CreateShaderResourceView(depthStencil, &shaderResourceViewDesc, &depthStencilSRV));
	}

	lastBoundUnit = -1;
	lastBoundDepthUnit = -1;

	FLOAT colors[4] = { 0, 0, 0, 0 };
	context->ClearRenderTargetView(renderTargetView[0], colors);
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

	texture = nullptr;
	if (!isDepthAttachment) {
		affirm(device->CreateTexture2D(&desc, nullptr, &texture));

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = desc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;

		for (int i = 0; i < 6; i++) {
			renderTargetView[i] = nullptr;
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			affirm(device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView[i]));
		}		
	}

	depthStencil = nullptr;
	depthStencilView = nullptr;
	DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (depthBufferBits == 32 && stencilBufferBits == 8) {
		depthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}
	else if (depthBufferBits == 16 && stencilBufferBits <= 0) {
		depthFormat = DXGI_FORMAT_D16_UNORM;
	}
	else if (stencilBufferBits <= 0) {
		depthFormat = DXGI_FORMAT_D32_FLOAT;
	}

	if (depthBufferBits > 0) {
		CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
		if (antialiasing) {
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}
		else {
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

		affirm(device->CreateDepthStencilView(depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT), &depthStencilView));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	if (!isDepthAttachment) {
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		affirm(device->CreateShaderResourceView(texture, &shaderResourceViewDesc, &renderTargetSRV));
	}

	if (depthBufferBits > 0) {
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		affirm(device->CreateShaderResourceView(depthStencil, &shaderResourceViewDesc, &depthStencilSRV));
	}

	lastBoundUnit = -1;
	lastBoundDepthUnit = -1;

	FLOAT colors[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < 6; i++) {
		context->ClearRenderTargetView(renderTargetView[i], colors);
	}
}

Graphics4::RenderTarget::~RenderTarget() {
	depthStencilView->Release();
	for (int i = 0; i < 6; i++) {
		renderTargetView[i]->Release();
	}
	renderTargetSRV->Release();
}

void Graphics4::RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, isDepthAttachment ? &depthStencilSRV : &renderTargetSRV);
	lastBoundUnit = unit.unit;
}

void Graphics4::RenderTarget::useDepthAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &depthStencilSRV);
	lastBoundDepthUnit = unit.unit;
}

void Graphics4::RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	depthStencil = source->depthStencil;
	depthStencilView = source->depthStencilView;
	depthStencilSRV = source->depthStencilSRV;
}

void Graphics4::RenderTarget::getPixels(u8* data) {}

void Graphics4::RenderTarget::generateMipmaps(int levels) {}
