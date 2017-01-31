#include "pch.h"

#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore;

Texture::Texture(const char* filename, bool readable) : Texture(FileReader(filename), filename, readable) {}

Texture::Texture(void* data, int size, const char* format, bool readable) : Texture(BufferReader(data, size), format, readable) {}
