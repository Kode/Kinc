#include "pch.h"
#include "Direct3D11.h"
#include "TextureImpl.h"

using namespace Kore;

namespace {
	void affirm(HRESULT) { }
	
	Texture* setTextures[16] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
									nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
}

Texture::Texture(const char* filename) : Image(filename) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = this->data;
	data.SysMemPitch = width * 4;
	data.SysMemSlicePitch = 0;

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, &data, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));
}

Texture::Texture(int width, int height, Format format) : Image(width, height, format) {
	stage = 0;
	mipmap = true;
}

TextureImpl::~TextureImpl() {
	unset();
	
}

void TextureImpl::unmipmap() {
	mipmap = false;
	
}

void Texture::set(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &view);
	this->stage = unit.unit;
	setTextures[stage] = this;
}

void TextureImpl::unset() {
	if (setTextures[stage] == this) {
		
		setTextures[stage] = nullptr;
	}
}
