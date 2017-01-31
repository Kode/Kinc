#include "pch.h"

#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore;

Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
	init(format, readable);
}

Texture::Texture(const char* filename, bool readable) : Image(FileReader(filename), filename, readable) {
	init(filename, readable);
}

Texture::Texture(void* data, int size, const char* format, bool readable) : Image(BufferReader(data, size), format, readable) {
	init(format, readable);
}
