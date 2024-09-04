#ifndef KOPE_G5_BUFFER_HEADER
#define KOPE_G5_BUFFER_HEADER

#include <kope/global.h>

#include "api.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/buffer_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/buffer_structs.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_buffer {
#ifdef KOPE_G5_VALIDATION
	uint32_t usage_flags;
#endif
	KOPE_G5_IMPL(buffer);
} kope_g5_buffer;

void kope_g5_buffer_destroy(kope_g5_buffer *buffer);
void *kope_g5_buffer_lock(kope_g5_buffer *buffer); // TODO
void kope_g5_buffer_unlock(kope_g5_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
