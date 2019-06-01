#include "pch.h"

#include <kinc/graphics5/constantbuffer.h>

#import <Metal/Metal.h>

id getMetalDevice();

bool kinc_g5_transposeMat3 = true;
bool kinc_g5_transposeMat4 = true;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = nullptr;
	buffer->impl._buffer = [getMetalDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	uint8_t *data = (uint8_t*)[buffer->impl._buffer contents];
	buffer->data = &data[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	
	buffer->data = nullptr;
}


int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}
