#include "pch.h"

#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore;

#ifdef KORE_G5ONG4
Graphics4::Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
	init(format, readable);
}

Graphics4::Texture::Texture(const char* filename, bool readable) {
	FileReader reader(filename);
	Image::init(reader, filename, readable);
	init(filename, readable);
}

Graphics4::Texture::Texture(void* data, int size, const char* format, bool readable) {
	BufferReader reader(data, size);
	Image::init(reader, format, readable);
	init(format, readable);
}

Graphics4::Texture::Texture(void* data, int width, int height, int format, bool readable) : Image(data, width, height, Image::Format(format), readable) {
	init("", readable);
}

Graphics4::Texture::Texture(void* data, int width, int height, int depth, int format, bool readable) : Image(data, width, height, depth, Image::Format(format), readable) {
	init3D(readable);
}
#endif
