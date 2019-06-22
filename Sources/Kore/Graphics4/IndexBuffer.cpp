#include "pch.h"

#include "Graphics.h"

using namespace Kore;
using namespace Kore::Graphics4;

IndexBuffer::IndexBuffer(int count) {
	kinc_g4_index_buffer_init(&kincBuffer, count);
}

IndexBuffer::~IndexBuffer() {
	kinc_g4_index_buffer_destroy(&kincBuffer);
}

int* IndexBuffer::lock() {
	return kinc_g4_index_buffer_lock(&kincBuffer);
}

void IndexBuffer::unlock() {
	kinc_g4_index_buffer_unlock(&kincBuffer);
}

int IndexBuffer::count() {
	return kinc_g4_index_buffer_count(&kincBuffer);
}

//void IndexBuffer::_set() {}
