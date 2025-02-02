#include "buffer_functions.h"

#include <kope/graphics5/buffer.h>

void kope_vulkan_buffer_set_name(kope_g5_buffer *buffer, const char *name) {}

void kope_vulkan_buffer_destroy(kope_g5_buffer *buffer) {}

void *kope_vulkan_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	return NULL;
}

void *kope_vulkan_buffer_lock_all(kope_g5_buffer *buffer) {
	return NULL;
}

void *kope_vulkan_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	return NULL;
}

void *kope_vulkan_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	return NULL;
}

void kope_vulkan_buffer_unlock(kope_g5_buffer *buffer) {}
