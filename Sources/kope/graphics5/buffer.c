#include "buffer.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/buffer_functions.h>
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

void *kope_g5_buffer_lock(kope_g5_buffer *buffer) {
	return KOPE_G5_CALL1(buffer_lock, buffer);
}

void kope_g5_buffer_unlock(kope_g5_buffer *buffer) {
	KOPE_G5_CALL1(buffer_unlock, buffer);
}
