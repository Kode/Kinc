#include "buffer.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/buffer_functions.h>
#endif

#ifdef KOPE_METAL
#include <kope/metal/buffer_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/buffer_functions.h>
#endif

void kope_g5_buffer_set_name(kope_g5_buffer *buffer, const char *name) {
	KOPE_G5_CALL2(buffer_set_name, buffer, name);
}

void kope_g5_buffer_destroy(kope_g5_buffer *buffer) {
	KOPE_G5_CALL1(buffer_destroy, buffer);
}

void *kope_g5_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	return KOPE_G5_CALL1(buffer_try_to_lock_all, buffer);
}

void *kope_g5_buffer_lock_all(kope_g5_buffer *buffer) {
	return KOPE_G5_CALL1(buffer_lock_all, buffer);
}

void *kope_g5_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	return KOPE_G5_CALL3(buffer_try_to_lock, buffer, offset, size);
}

void *kope_g5_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	return KOPE_G5_CALL3(buffer_lock, buffer, offset, size);
}

void kope_g5_buffer_unlock(kope_g5_buffer *buffer) {
	KOPE_G5_CALL1(buffer_unlock, buffer);
}
