#include "pch.h"

#include "Graphics.h"

using namespace Kore;
using namespace Kore::Graphics4;

IndexBuffer::IndexBuffer(int count) {
	Kinc_G4_IndexBuffer_Create(&kincBuffer, count);
}

IndexBuffer::~IndexBuffer() {
	Kinc_G4_IndexBuffer_Destroy(&kincBuffer);
}

int* IndexBuffer::lock() {
	return Kinc_G4_IndexBuffer_Lock(&kincBuffer);
}

void IndexBuffer::unlock() {
	Kinc_G4_IndexBuffer_Unlock(&kincBuffer);
}

int IndexBuffer::count() {
	return Kinc_G4_IndexBuffer_Count(&kincBuffer);
}

//void IndexBuffer::_set() {}
