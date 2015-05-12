#include "pch.h"
#include "Direct3D12.h"
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

}

TextureImpl::~TextureImpl() {
	unset();
	
}

void TextureImpl::unmipmap() {
	mipmap = false;
	
}

void Texture::set(TextureUnit unit) {
	if (unit.unit < 0) return;
	//context->PSSetShaderResources(unit.unit, 1, &view);
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
	//context->UpdateSubresource(texture, 0, nullptr, data, 0, 0);
}

int Texture::stride() {
	return 1;
}
