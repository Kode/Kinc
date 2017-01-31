#include <string.h>
#include "pch.h"
#include "BufferReader.h"

namespace Kore {

	BufferReader::BufferReader(void const* buffer, int size) : buffer((u8*)buffer), bufferSize(size) {
	}

	BufferReader::~BufferReader() {
		if (readAllBuffer != nullptr)
			delete[] readAllBuffer;
	}

	int BufferReader::read(void* data, int size) {
		int bytesAvailable = size - position;
		if (size > bytesAvailable) size = bytesAvailable;
		memcpy(data, buffer + position, size);
		position += size;
		return size;
	}

	// create a copy of the buffer, because returned buffer can be changed...
	void* BufferReader::readAll() {
		if (readAllBuffer != nullptr) 
			delete[] readAllBuffer;
		readAllBuffer = new Kore::u8[bufferSize];
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