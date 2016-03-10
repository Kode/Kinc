#include "pch.h"
#include "Direct3D11.h"
#include "TextureImpl.h"
#include <Kore/WinError.h>

using namespace Kore;

namespace {
	Texture* setTextures[16] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
									nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
}

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
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

	if (!readable) {
		delete[] this->data;
		this->data = nullptr;
	}
}

Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, nullptr, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));
}

TextureImpl::~TextureImpl() {
	unset();
	
}

void TextureImpl::unmipmap() {
	mipmap = false;
	
}

void Texture::_set(TextureUnit unit) {
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

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	context->UpdateSubresource(texture, 0, nullptr, data, stride(), 0);
}

int Texture::stride() {
	return format == Image::RGBA32 ? width * 4 : width;
}

void Texture::generateMipmaps(int levels) {

}

void Texture::setMipmap(Texture* mipmap, int level) {

}
