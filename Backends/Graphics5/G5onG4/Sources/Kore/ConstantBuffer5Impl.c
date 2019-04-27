#include "pch.h"

#include <Kinc/Graphics5/ConstantBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

const bool kinc_g5_internal_transposeMat3 = false;
const bool kinc_g5_internal_transposeMat4 = false;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

#ifdef __cplusplus
}
#endif
