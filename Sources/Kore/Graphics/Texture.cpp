#include "pch.h"

#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore;

Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
	init(format, readable);
}

Texture::Texture(const char* filename, bool readable) {
	FileReader reader(filename);
	Image::init(reader, filename, readable);
	init(filename, readable);
}

Texture::Texture(void* data, int size, const char* format, bool readable) {
	BufferReader reader(data, size);
	Image::init(reader, format, readable);
	init(format, readable);
}
