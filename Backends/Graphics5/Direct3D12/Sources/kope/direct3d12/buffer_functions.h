#ifndef KOPE_D3D12_BUFFER_FUNCTIONS_HEADER
#define KOPE_D3D12_BUFFER_FUNCTIONS_HEADER

#include <kope/graphics5/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_buffer_set_name(kope_g5_buffer *buffer, const char *name);

void kope_d3d12_buffer_destroy(kope_g5_buffer *buffer);

void *kope_d3d12_buffer_try_to_lock_all(kope_g5_buffer *buffer);

void *kope_d3d12_buffer_lock_all(kope_g5_buffer *buffer);

void *kope_d3d12_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size);

void *kope_d3d12_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size);

void kope_d3d12_buffer_unlock(kope_g5_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
