#include "Graphics.h"

using namespace Kore;
using namespace Kore::Graphics4;

IndexBuffer::IndexBuffer(int count, Usage usage) {
#if defined(KORE_ANDROID) || defined(KORE_PI)
	kinc_g4_index_buffer_init(&kincBuffer, count, KINC_G4_INDEX_BUFFER_FORMAT_16BIT, (kinc_g4_usage_t)usage);
#else
	kinc_g4_index_buffer_init(&kincBuffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, (kinc_g4_usage_t)usage);
#endif
}

IndexBuffer::~IndexBuffer() {
	kinc_g4_index_buffer_destroy(&kincBuffer);
}

int *IndexBuffer::lock() {
	return kinc_g4_index_buffer_lock(&kincBuffer);
}

void IndexBuffer::unlock() {
	kinc_g4_index_buffer_unlock(&kincBuffer);
}

int IndexBuffer::count() {
	return kinc_g4_index_buffer_count(&kincBuffer);
}

// void IndexBuffer::_set() {}
