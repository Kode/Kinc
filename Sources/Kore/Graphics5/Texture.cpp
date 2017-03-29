#include "pch.h"

#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore;

Graphics5::Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
	_init(format, readable);
}

Graphics5::Texture::Texture(const char* filename, bool readable) {
	FileReader reader(filename);
	Image::init(reader, filename, readable);
	_init(filename, readable);
}

Graphics5::Texture::Texture(void* data, int size, const char* format, bool readable) {
	BufferReader reader(data, size);
	Image::init(reader, format, readable);
	_init(format, readable);
}

Graphics5::Texture::Texture(void* data, int width, int height, int format, bool readable) : Image(data, width, height, Image::Format(format), readable) {
	_init("", readable);
}
