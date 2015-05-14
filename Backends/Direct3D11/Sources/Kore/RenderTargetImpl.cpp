#include "pch.h"
#include "Direct3D11.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, bool zBuffer, bool antialiasing, RenderTargetFormat format) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, nullptr, &texture));

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = desc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	affirm(device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	affirm(device->CreateShaderResourceView(texture, &shaderResourceViewDesc, &view));
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &view);
}
