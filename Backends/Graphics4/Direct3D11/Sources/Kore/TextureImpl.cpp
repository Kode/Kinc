#include "pch.h"

#include "Direct3D11.h"
#include "TextureImpl.h"
#include <Kore/Math/Random.h>
#include <Kore/SystemMicrosoft.h>

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

	bool isHdr(Graphics4::Image::Format format) {
		return format == Graphics4::Image::RGBA128 || format == Graphics4::Image::RGBA64 ||
			   format == Graphics4::Image::A32 || format == Graphics4::Image::A16;
	}
}

void Graphics4::Texture::init(const char* format, bool readable) {
	setId();
	stage = 0;
	texWidth = width;
	texHeight = height;
	rowPitch = 0;

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
	data.pSysMem = isHdr(this->format) ? (void*)this->hdrData : this->data;
	data.SysMemPitch = width * formatByteSize(this->format);
	data.SysMemSlicePitch = 0;

	texture = nullptr;
	Kore_Microsoft_Affirm(device->CreateTexture2D(&desc, &data, &texture));
	Kore_Microsoft_Affirm(device->CreateShaderResourceView(texture, nullptr, &view));

	if (!readable) {
		if (isHdr(this->format)) {
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
	setId();
	stage = 0;
	texWidth = width;
	texHeight = height;
	texDepth = depth;
	rowPitch = 0;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;
	desc.Format = convertFormat(this->format);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = isHdr(this->format) ? (void*)this->hdrData : this->data;
	data.SysMemPitch = width * formatByteSize(this->format);
	data.SysMemSlicePitch = width * height * formatByteSize(this->format);

	texture3D = nullptr;
	Kore_Microsoft_Affirm(device->CreateTexture3D(&desc, &data, &texture3D));
	Kore_Microsoft_Affirm(device->CreateShaderResourceView(texture3D, nullptr, &view));

	if (!readable) {
		if (isHdr(this->format)) {
			delete[] this->hdrData;
			this->hdrData = nullptr;
		}
		else {
			delete[] this->data;
			this->data = nullptr;
		}
	}
}

Graphics4::Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	stage = 0;
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
	Kore_Microsoft_Affirm(device->CreateTexture2D(&desc, nullptr, &texture));
	Kore_Microsoft_Affirm(device->CreateShaderResourceView(texture, nullptr, &view));

	if (format == Image::RGBA128) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = desc.Format;
		du.Texture2D.MipSlice = 0;
		du.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
		Kore_Microsoft_Affirm(device->CreateUnorderedAccessView(texture, &du, &computeView));
	}
}

Graphics4::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {
	stage = 0;
	texWidth = width;
	texHeight = height;
	texDepth = depth;
	hasMipmaps = true;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.Format = format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	texture3D = nullptr;
	Kore_Microsoft_Affirm(device->CreateTexture3D(&desc, nullptr, &texture3D));
	Kore_Microsoft_Affirm(device->CreateShaderResourceView(texture3D, nullptr, &view));
}

TextureImpl::TextureImpl() : hasMipmaps(false), renderView(nullptr), computeView(nullptr) {}

TextureImpl::~TextureImpl() {
	unset();
	if (view != nullptr) {
		view->Release();
	}
	if (texture != nullptr) {
		texture->Release();
	}
	if (computeView != nullptr) {
		computeView->Release();
	}
}

void TextureImpl::unmipmap() {
	hasMipmaps = false;
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

void Graphics4::Texture::_setImage(TextureUnit unit) {
	if (unit.unit < 0) return;
	if (computeView == nullptr) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
		du.Texture3D.MipSlice = 0;
		du.Texture3D.FirstWSlice = 0;
		du.Texture3D.WSize = -1;
		du.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE3D;
		Kore_Microsoft_Affirm(device->CreateUnorderedAccessView(texture3D, &du, &computeView));
	}
	context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, unit.unit, 1, &computeView, nullptr);
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
	if (renderView == nullptr) {
		texDepth > 1 ? 
			Kore_Microsoft_Affirm(device->CreateRenderTargetView(texture3D, 0, &renderView)) :
			Kore_Microsoft_Affirm(device->CreateRenderTargetView(texture, 0, &renderView));
	}
	static float clearColor[4];
	clearColor[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clearColor[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
	clearColor[2] = (color & 0x000000ff) / 255.0f;
	clearColor[3] = ((color & 0xff000000) >> 24) / 255.0f;
	context->ClearRenderTargetView(renderView, clearColor);
}

int Graphics4::Texture::stride() {
	return rowPitch;
}

void TextureImpl::enableMipmaps(int texWidth, int texHeight, int format) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = texWidth;
	desc.Height = texHeight;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = convertFormat((Graphics4::Image::Format)format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* mipMappedTexture;
	ID3D11ShaderResourceView* mipMappedView;
	Kore_Microsoft_Affirm(device->CreateTexture2D(&desc, nullptr, &mipMappedTexture));
	Kore_Microsoft_Affirm(device->CreateShaderResourceView(mipMappedTexture, nullptr, &mipMappedView));

	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.right = texWidth;
	sourceRegion.top = 0;
	sourceRegion.bottom = texHeight;
	sourceRegion.front = 0;
	sourceRegion.back = 1;
	context->CopySubresourceRegion(mipMappedTexture, 0, 0, 0, 0, texture, 0, &sourceRegion);

	if (texture != nullptr) texture->Release();
	texture = mipMappedTexture;

	if (view != nullptr) view->Release();
	view = mipMappedView;

	hasMipmaps = true;
}

void Graphics4::Texture::generateMipmaps(int levels) {
	if (!hasMipmaps) enableMipmaps(texWidth, texHeight, format);
	context->GenerateMips(view);
}

void Graphics4::Texture::setMipmap(Texture* mipmap, int level) {
	if (!hasMipmaps) enableMipmaps(texWidth, texHeight, format);
	D3D11_BOX dstRegion;
	dstRegion.left = 0;
	dstRegion.right = mipmap->texWidth;
	dstRegion.top = 0;
	dstRegion.bottom = mipmap->texHeight;
	dstRegion.front = 0;
	dstRegion.back = 1;
	context->UpdateSubresource(texture, level, &dstRegion, isHdr(mipmap->format) ? (void*)mipmap->hdrData : mipmap->data, mipmap->texWidth * formatByteSize(mipmap->format), 0);
}
