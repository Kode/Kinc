#include "pch.h"

#include "Direct3D11.h"
#include "TextureImpl.h"
#include <Kore/Math/Random.h>
#include <Kore/WinError.h>

using namespace Kore;

namespace {
	Texture* setTextures[16] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
}

void Texture::init(const char* format, bool readable) {
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
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
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
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;

	if (format == Image::RGBA128) { // for compute
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
	}
	else {
		desc.Format = format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, nullptr, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));

	computeView = nullptr;
	if (format == Image::RGBA128) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = desc.Format;
		du.Texture2D.MipSlice = 0;
		du.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
		affirm(device->CreateUnorderedAccessView(texture, &du, &computeView));
	}
}

Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

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
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	return (u8*)mappedResource.pData;
}

void Texture::unlock() {
	context->Unmap(texture, 0);
}

int Texture::stride() {
	return format == Image::RGBA32 ? width * 4 : width; // TODO: Return mappedResource's stride
}

void Texture::generateMipmaps(int levels) {}

void Texture::setMipmap(Texture* mipmap, int level) {}
