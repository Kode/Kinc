#include "pch.h"

#include "BufferReader.h"

#include <stdlib.h>
#include <string.h>

namespace Kore {

	BufferReader::BufferReader(void const* buffer, int size) : buffer((u8*)buffer), bufferSize(size), position(0), readAllBuffer(nullptr) {}

	BufferReader::~BufferReader() {
		if (readAllBuffer != nullptr)
			free(readAllBuffer);
	}

	int BufferReader::read(void* data, int size) {
		int bytesAvailable = bufferSize - position;
		if (size > bytesAvailable) size = bytesAvailable;
		memcpy(data, buffer + position, size);
		position += size;
		return size;
	}

	// create a copy of the buffer, because returned buffer can be changed...
	void* BufferReader::readAll() {
		if (readAllBuffer != nullptr) 
			free(readAllBuffer);
		readAllBuffer = malloc(bufferSize);
		memcpy(readAllBuffer, buffer, bufferSize);
		return readAllBuffer;
	}

	int BufferReader::size() const {
		return bufferSize;
	}

	int BufferReader::pos() const {
		return position;
	}

	void BufferReader::seek(int pos) {
		position = pos < 0 ? 0 : (pos > bufferSize ? bufferSize : pos);
	}
}
