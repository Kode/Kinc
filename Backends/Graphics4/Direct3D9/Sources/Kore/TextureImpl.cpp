#include "pch.h"

#include "Direct3D9.h"
#include "TextureImpl.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/WinError.h>

using namespace Kore;

namespace {
	Graphics4::Texture* setTextures[16] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

	D3DFORMAT convert(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA32:
		default:
			return D3DFMT_A8R8G8B8;
		case Graphics4::Image::Grey8:
			return D3DFMT_L8;
		}
	}
}

void Graphics4::Texture::init(const char* format, bool readable) {
	stage = 0;
	mipmap = true;
	DWORD usage = 0;
	texWidth = width;
	texHeight = height;
	usage = D3DUSAGE_DYNAMIC;
	affirm(device->CreateTexture(width, height, 1, usage, convert(this->format), D3DPOOL_DEFAULT, &texture, 0), "Texture creation failed.");
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	pitch = rect.Pitch;
	u8* from = (u8*)data;
	u8* to = (u8*)rect.pBits;
	// memcpy(to, from, width * height * sizeOf(format));
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			to[rect.Pitch * y + x * 4 + 0 /* blue*/] = (from[y * width * 4 + x * 4 + 2]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 1 /*green*/] = (from[y * width * 4 + x * 4 + 1]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 2 /*  red*/] = (from[y * width * 4 + x * 4 + 0]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 3 /*alpha*/] = (from[y * width * 4 + x * 4 + 3]); /// 255.0f;
		}
	}
	affirm(texture->UnlockRect(0));
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

void Graphics4::Texture::init3D(bool readable) {
}

Graphics4::Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	stage = 0;
	mipmap = true;
	DWORD usage = 0;
	texWidth = width;
	texHeight = height;
	usage = D3DUSAGE_DYNAMIC;
	affirm(device->CreateTexture(width, height, 1, usage, convert(format), D3DPOOL_DEFAULT, &texture, 0), "Texture creation failed.");
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

Graphics4::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

TextureImpl::~TextureImpl() {
	unset();
	texture->Release();
}

void Graphics4::Texture::_set(TextureUnit unit) {
	affirm(device->SetTexture(unit.unit, texture));
	this->stage = unit.unit;
	setTextures[stage] = this;
}

void TextureImpl::unset() {
	if (setTextures[stage] == (void*)this) {
		device->SetTexture(stage, nullptr);
		setTextures[stage] = nullptr;
	}
}

u8* Graphics4::Texture::lock() {
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	pitch = rect.Pitch;
	return (u8*)rect.pBits;
}

void Graphics4::Texture::unlock() {
	affirm(texture->UnlockRect(0));
}

void Graphics4::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

int Graphics4::Texture::stride() {
	return pitch;
}

void Graphics4::Texture::generateMipmaps(int levels) {}

void Graphics4::Texture::setMipmap(Texture* mipmap, int level) {}
