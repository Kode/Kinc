#include "pch.h"

#include "Direct3D11.h"

#include <Kinc/Graphics4/Texture.h>
#include <Kinc/Graphics4/TextureUnit.h>

#include <Kore/Math/Random.h>
#include <Kore/SystemMicrosoft.h>

#include <assert.h>

using namespace Kore;

static Kinc_G4_Texture *setTextures[16] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

static DXGI_FORMAT convertFormat(Kinc_ImageFormat format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_IMAGE_FORMAT_RGB24:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case KINC_IMAGE_FORMAT_A32:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_IMAGE_FORMAT_A16:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_IMAGE_FORMAT_GREY8:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_BGRA32:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		assert(false);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(Kinc_ImageFormat format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		assert(false);
		return 4;
	}
}

static bool isHdr(Kinc_ImageFormat format) {
	return format == KINC_IMAGE_FORMAT_RGBA128 || format == KINC_IMAGE_FORMAT_RGBA64 || format == KINC_IMAGE_FORMAT_A32 || format == KINC_IMAGE_FORMAT_A16;
}

static void init_texture(Kinc_G4_Texture *texture, Kinc_ImageFormat format, bool readable) {
	texture->impl.stage = 0;
	texture->texWidth = texture->image.width;
	texture->texHeight = texture->image.height;
	texture->impl.rowPitch = 0;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = texture->image.width;
	desc.Height = texture->image.height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = convertFormat(texture->image.format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = isHdr(texture->image.format) ? (void*)texture->image.hdrData : texture->image.data;
	data.SysMemPitch = texture->image.width * formatByteSize(texture->image.format);
	data.SysMemSlicePitch = 0;

	texture->impl.texture = nullptr;
	Kinc_Microsoft_Affirm(device->CreateTexture2D(&desc, &data, &texture->impl.texture));
	Kinc_Microsoft_Affirm(device->CreateShaderResourceView(texture->impl.texture, nullptr, &texture->impl.view));

	if (!readable) {
		if (isHdr(texture->image.format)) {
			delete[] texture->image.hdrData;
			texture->image.hdrData = nullptr;
		}
		else {
			delete[] texture->image.data;
			texture->image.data = nullptr;
		}
	}
}

static void init_texture3d(Kinc_G4_Texture *texture, bool readable) {
	texture->impl.stage = 0;
	texture->texWidth = texture->image.width;
	texture->texHeight = texture->image.height;
	texture->texDepth = texture->image.depth;
	texture->impl.rowPitch = 0;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = texture->image.width;
	desc.Height = texture->image.height;
	desc.Depth = texture->image.depth;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;
	desc.Format = convertFormat(texture->image.format);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = isHdr(texture->image.format) ? (void*)texture->image.hdrData : texture->image.data;
	data.SysMemPitch = texture->image.width * formatByteSize(texture->image.format);
	data.SysMemSlicePitch = texture->image.width * texture->image.height * formatByteSize(texture->image.format);

	texture->impl.texture3D = nullptr;
	Kinc_Microsoft_Affirm(device->CreateTexture3D(&desc, &data, &texture->impl.texture3D));
	Kinc_Microsoft_Affirm(device->CreateShaderResourceView(texture->impl.texture3D, nullptr, &texture->impl.view));

	if (!readable) {
		if (isHdr(texture->image.format)) {
			delete[] texture->image.hdrData;
			texture->image.hdrData = nullptr;
		}
		else {
			delete[] texture->image.data;
			texture->image.data = nullptr;
		}
	}
}

void Kinc_G4_Texture_Create(Kinc_G4_Texture *texture, int width, int height, Kinc_ImageFormat format, bool readable) {
	init_texture(texture, format, readable);
	texture->impl.stage = 0;
	texture->texWidth = width;
	texture->texHeight = height;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;

	if (format == KINC_IMAGE_FORMAT_RGBA128) { // for compute
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
	}
	else {
		desc.Format = format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	texture->impl.texture = nullptr;
	Kinc_Microsoft_Affirm(device->CreateTexture2D(&desc, nullptr, &texture->impl.texture));
	Kinc_Microsoft_Affirm(device->CreateShaderResourceView(texture->impl.texture, nullptr, &texture->impl.view));

	if (format == KINC_IMAGE_FORMAT_RGBA128) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = desc.Format;
		du.Texture2D.MipSlice = 0;
		du.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
		Kinc_Microsoft_Affirm(device->CreateUnorderedAccessView(texture->impl.texture, &du, &texture->impl.computeView));
	}
}

void Kinc_G4_TextureCreate3D(Kinc_G4_Texture *texture, int width, int height, int depth, Kinc_ImageFormat format, bool readable) {
	init_texture3d(texture, readable);
	texture->impl.stage = 0;
	texture->texWidth = width;
	texture->texHeight = height;
	texture->texDepth = depth;
	texture->impl.hasMipmaps = true;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.Format = format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	texture->impl.texture3D = nullptr;
	Kinc_Microsoft_Affirm(device->CreateTexture3D(&desc, nullptr, &texture->impl.texture3D));
	Kinc_Microsoft_Affirm(device->CreateShaderResourceView(texture->impl.texture3D, nullptr, &texture->impl.view));
}

//TextureImpl::TextureImpl() : hasMipmaps(false), renderView(nullptr), computeView(nullptr) {}

void Kinc_Internal_TextureUnset(Kinc_G4_Texture *texture);

void Kinc_G4_Texture_Destroy(Kinc_G4_Texture *texture) {
	Kinc_Internal_TextureUnset(texture);
	if (texture->impl.view != nullptr) {
		texture->impl.view->Release();
	}
	if (texture->impl.texture != nullptr) {
		texture->impl.texture->Release();
	}
	if (texture->impl.computeView != nullptr) {
		texture->impl.computeView->Release();
	}
}

void Kinc_Internal_TextureUnmipmap(Kinc_G4_Texture *texture) {
	texture->impl.hasMipmaps = false;
}

void Kinc_Internal_TextureSet(Kinc_G4_Texture *texture, Kinc_G4_TextureUnit unit) {
	if (unit.impl.unit < 0) return;
	if (unit.impl.vertex) {
		context->VSSetShaderResources(unit.impl.unit, 1, &texture->impl.view);
	}
	else {
		context->PSSetShaderResources(unit.impl.unit, 1, &texture->impl.view);
	}
	texture->impl.stage = unit.impl.unit;
	setTextures[texture->impl.stage] = texture;
}

void Kinc_Internal_TexureSetImage(Kinc_G4_Texture *texture, Kinc_G4_TextureUnit unit) {
	if (unit.impl.unit < 0) return;
	if (texture->impl.computeView == nullptr) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = texture->image.format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
		du.Texture3D.MipSlice = 0;
		du.Texture3D.FirstWSlice = 0;
		du.Texture3D.WSize = -1;
		du.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE3D;
		Kinc_Microsoft_Affirm(device->CreateUnorderedAccessView(texture->impl.texture3D, &du, &texture->impl.computeView));
	}
	context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, unit.impl.unit, 1, &texture->impl.computeView, nullptr);
}

void Kinc_Internal_TextureUnset(Kinc_G4_Texture *texture) {
	if (setTextures[texture->impl.stage] == texture) {

		setTextures[texture->impl.stage] = NULL;
	}
}

u8 *Kinc_G4_Texture_Lock(Kinc_G4_Texture *texture) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(texture->impl.texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	texture->impl.rowPitch = mappedResource.RowPitch;
	return (u8*)mappedResource.pData;
}

void Kinc_G4_Texture_Unlock(Kinc_G4_Texture *texture) {
	context->Unmap(texture->impl.texture, 0);
}

void Kinc_G4_Texture_Clear(Kinc_G4_Texture *texture, int x, int y, int z, int width, int height, int depth, uint color) {
	if (texture->impl.renderView == nullptr) {
		texture->texDepth > 1 ? 
			Kinc_Microsoft_Affirm(device->CreateRenderTargetView(texture->impl.texture3D, 0, &texture->impl.renderView))
		             : Kinc_Microsoft_Affirm(device->CreateRenderTargetView(texture->impl.texture, 0, &texture->impl.renderView));
	}
	static float clearColor[4];
	clearColor[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clearColor[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
	clearColor[2] = (color & 0x000000ff) / 255.0f;
	clearColor[3] = ((color & 0xff000000) >> 24) / 255.0f;
	context->ClearRenderTargetView(texture->impl.renderView, clearColor);
}

int Kinc_G4_Texture_Stride(Kinc_G4_Texture *texture) {
	return texture->impl.rowPitch;
}

static void enableMipmaps(Kinc_G4_Texture *texture, int texWidth, int texHeight, int format) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = texWidth;
	desc.Height = texHeight;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = convertFormat((Kinc_ImageFormat)format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* mipMappedTexture;
	ID3D11ShaderResourceView* mipMappedView;
	Kinc_Microsoft_Affirm(device->CreateTexture2D(&desc, nullptr, &mipMappedTexture));
	Kinc_Microsoft_Affirm(device->CreateShaderResourceView(mipMappedTexture, nullptr, &mipMappedView));

	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.right = texWidth;
	sourceRegion.top = 0;
	sourceRegion.bottom = texHeight;
	sourceRegion.front = 0;
	sourceRegion.back = 1;
	context->CopySubresourceRegion(mipMappedTexture, 0, 0, 0, 0, texture->impl.texture, 0, &sourceRegion);

	if (texture->impl.texture != nullptr) {
		texture->impl.texture->Release();
	}
	texture->impl.texture = mipMappedTexture;

	if (texture->impl.view != nullptr) {
		texture->impl.view->Release();
	}
	texture->impl.view = mipMappedView;

	texture->impl.hasMipmaps = true;
}

void Kinc_G4_Texture_GenerateMipmaps(Kinc_G4_Texture *texture, int levels) {
	if (!texture->impl.hasMipmaps) {
		enableMipmaps(texture, texture->texWidth, texture->texHeight, texture->image.format);
	}
	context->GenerateMips(texture->impl.view);
}

void Kinc_G4_Texture_setMipmap(Kinc_G4_Texture *texture, Kinc_G4_Texture *mipmap, int level) {
	if (!texture->impl.hasMipmaps) {
		enableMipmaps(texture, texture->texWidth, texture->texHeight, texture->image.format);
	}
	D3D11_BOX dstRegion;
	dstRegion.left = 0;
	dstRegion.right = mipmap->texWidth;
	dstRegion.top = 0;
	dstRegion.bottom = mipmap->texHeight;
	dstRegion.front = 0;
	dstRegion.back = 1;
	context->UpdateSubresource(texture->impl.texture, level, &dstRegion, isHdr(mipmap->image.format) ? (void*)mipmap->image.hdrData : mipmap->image.data, mipmap->texWidth * formatByteSize(mipmap->image.format), 0);
}
