#include "pch.h"

#include <Kore/Graphics5/ConstantBuffer.h>

using namespace Kore;

Graphics5::ConstantBuffer::ConstantBuffer(int size) {
	mySize = size;
	data = nullptr;

}

Graphics5::ConstantBuffer::~ConstantBuffer() {}

void Graphics5::ConstantBuffer::lock() {
	lock(0, size());
}

void Graphics5::ConstantBuffer::lock(int start, int count) {
	lastStart = start;
	lastCount = count;

}

void Graphics5::ConstantBuffer::unlock() {

	data = nullptr;
}

int Graphics5::ConstantBuffer::size() {
	return mySize;
}
