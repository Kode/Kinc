#include "pch.h"

#include "Direct3D11.h"
#include "TextureImpl.h"
#include <Kore/Math/Random.h>
#include <Kore/WinError.h>

using namespace Kore;

namespace {
	Graphics4::Texture* setTextures[16] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

	DXGI_FORMAT convertFormat(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::BGRA32:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case Graphics4::Image::RGBA128:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Graphics4::Image::RGBA64:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case Graphics4::Image::RGB24:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Graphics4::Image::A32:
			return DXGI_FORMAT_R32_FLOAT;
		case Graphics4::Image::A16:
			return DXGI_FORMAT_R16_FLOAT;
		case Graphics4::Image::Grey8:
			return DXGI_FORMAT_R8_UNORM;
		case Graphics4::Image::RGBA32:
		default:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}

	int formatByteSize(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA128:
			return 16;
		case Graphics4::Image::RGBA64:
			return 8;
		case Graphics4::Image::RGB24:
			return 4;
		case Graphics4::Image::A32:
			return 4;
		case Graphics4::Image::A16:
			return 2;
		case Graphics4::Image::Grey8:
			return 1;
		case Graphics4::Image::BGRA32:
		case Graphics4::Image::RGBA32:
		default:
			return 4;
		}
	}
}

void Graphics4::Texture::init(const char* format, bool readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;
	rowPitch = 0;
	bool isHdr = this->format == Graphics4::Image::RGBA128 || this->format == Graphics4::Image::RGBA64 || this->format == Graphics4::Image::A32 || this->format == Graphics4::Image::A16;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = convertFormat(this->format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = isHdr ? (void*)this->hdrData : this->data;
	data.SysMemPitch = width * formatByteSize(this->format);
	data.SysMemSlicePitch = 0;

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, &data, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));

	computeView = nullptr;

	if (!readable) {
		if (isHdr) {
			delete[] this->hdrData;
			this->hdrData = nullptr;
		}
		else {
			delete[] this->data;
			this->data = nullptr;
		}
	}
}

void Graphics4::Texture::init3D(bool readable) {
}

Graphics4::Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
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

Graphics4::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

TextureImpl::~TextureImpl() {
	unset();
	if (view != nullptr) {
		view->Release();
	}
	if (texture != nullptr) {
		texture->Release();
	}
	if(computeView!=nullptr){
		computeView->Release();
	}
}

void TextureImpl::unmipmap() {
	mipmap = false;
}

void Graphics4::Texture::_set(TextureUnit unit) {
	if (unit.unit < 0) return;
	if (unit.vertex) {
		context->VSSetShaderResources(unit.unit, 1, &view);
	}
	else {
		context->PSSetShaderResources(unit.unit, 1, &view);
	}
	this->stage = unit.unit;
	setTextures[stage] = this;
}

void TextureImpl::unset() {
	if (setTextures[stage] == this) {

		setTextures[stage] = nullptr;
	}
}

u8* Graphics4::Texture::lock() {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	rowPitch = mappedResource.RowPitch;
	return (u8*)mappedResource.pData;
}

void Graphics4::Texture::unlock() {
	context->Unmap(texture, 0);
}

void Graphics4::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

int Graphics4::Texture::stride() {
	return rowPitch;
}

void Graphics4::Texture::generateMipmaps(int levels) {
	//context->GenerateMips(view);
}

void Graphics4::Texture::setMipmap(Texture* mipmap, int level) {
	//D3D11CalcSubresource();
	//context->UpdateSubresource();
}
