#include "pch.h"
#include "TextureImpl.h"
#include <Kore/WinError.h>
#include "Direct3D9.h"

using namespace Kore;

namespace {
	Texture* setTextures[16] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
									nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	D3DFORMAT convert(Image::Format format) {
		switch (format) {
		case Image::RGBA32:
		default:
			return D3DFMT_A8R8G8B8;
		case Image::Grey8:
			return D3DFMT_L8;
		}
	}
}

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
	stage = 0;
	mipmap = true;
	DWORD usage = 0;
	texWidth = width;
	texHeight = height;
	usage = D3DUSAGE_DYNAMIC;
	affirm(device->CreateTexture(width, height, 1, usage, convert(format), D3DPOOL_DEFAULT, &texture, 0), "Texture creation failed.");
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	pitch = rect.Pitch;
	u8* from = (u8*)data;
	u8* to = (u8*)rect.pBits;
	//memcpy(to, from, width * height * sizeOf(format));
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			to[rect.Pitch * y + x * 4 + 0 /* blue*/] = (from[y * width * 4 + x * 4 + 2]);///255.0f;
			to[rect.Pitch * y + x * 4 + 1 /*green*/] = (from[y * width * 4 + x * 4 + 1]);///255.0f;
			to[rect.Pitch * y + x * 4 + 2 /*  red*/] = (from[y * width * 4 + x * 4 + 0]);///255.0f;
			to[rect.Pitch * y + x * 4 + 3 /*alpha*/] = (from[y * width * 4 + x * 4 + 3]);///255.0f;
		}
	}
	affirm(texture->UnlockRect(0));
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
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

TextureImpl::~TextureImpl() {
	unset();
	texture->Release();
}

void Texture::_set(TextureUnit unit) {
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

u8* Texture::lock() {
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	pitch = rect.Pitch;
	return (u8*)rect.pBits;
}

void Texture::unlock() {
	affirm(texture->UnlockRect(0));
}

int Texture::stride() {
	return pitch;
}
