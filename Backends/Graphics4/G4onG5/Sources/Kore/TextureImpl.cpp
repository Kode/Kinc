#include "pch.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

#include "TextureImpl.h"

using namespace Kore;

Graphics4::Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
	_texture = new Graphics5::Texture(reader, format, readable);
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

Graphics4::Texture::Texture(const char* filename, bool readable) {
	_texture = new Graphics5::Texture(filename, readable);
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

Graphics4::Texture::Texture(void* data, int size, const char* format, bool readable) {
	_texture = new Graphics5::Texture(data, size, format, readable);
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

Graphics4::Texture::Texture(void* data, int width, int height, int format, bool readable) : Image(data, width, height, Image::Format(format), readable) {
	_texture = new Graphics5::Texture(data, width, height, format, readable);
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

Graphics4::Texture::Texture(void* data, int width, int height, int depth, int format, bool readable) : Image(data, width, height, depth, Image::Format(format), readable) {

}

void Graphics4::Texture::init(const char* format, bool readable) {
	_texture->_init(format, readable);
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

void Graphics4::Texture::init3D(bool readable) {
}

Graphics4::Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable), TextureImpl(width, height, format, readable) {
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

Graphics4::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable), TextureImpl(width, height, depth, format, readable) {
	width = _texture->width;
	height = _texture->height;
	texWidth = _texture->texWidth;
	texHeight = _texture->texHeight;
	data = _texture->data;
}

TextureImpl::TextureImpl() : _texture(nullptr), _uploaded(false) {
	// TODO
}

TextureImpl::TextureImpl(int width, int height, Kore::Image::Format format, bool readable) : _uploaded(false) {
	_texture = new Graphics5::Texture(width, height, format, readable);
	width = _texture->width;
	height = _texture->height;
	//texWidth = _texture->texWidth;
	//texHeight = _texture->texHeight;
}

TextureImpl::TextureImpl(int width, int height, int depth, Image::Format format, bool readable) : _uploaded(false) {
	_texture = new Graphics5::Texture(width, height, depth, format, readable);
	width = _texture->width;
	height = _texture->height;
	//texWidth = _texture->texWidth;
	//texHeight = _texture->texHeight;
}

TextureImpl::~TextureImpl() {
	unset();
}

void TextureImpl::unset() {
	// TODO
}

void TextureImpl::unmipmap() {
	// TODO
}

u8* Graphics4::Texture::lock() {
	return _texture->lock();
}

void Graphics4::Texture::unlock() {
	_texture->unlock();
}

void Graphics4::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {
	_texture->clear(x, y, z, width, height, depth, color);
}

int Graphics4::Texture::stride() {
	return _texture->stride();
}

void Graphics4::Texture::generateMipmaps(int levels) {
	_texture->generateMipmaps(levels);
}

void Graphics4::Texture::setMipmap(Texture* mipmap, int level) {
	_texture->setMipmap(mipmap->_texture, level);
}
