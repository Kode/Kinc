#ifndef KOPE_D3D12_BUFFER_FUNCTIONS_HEADER
#define KOPE_D3D12_BUFFER_FUNCTIONS_HEADER

#include <kope/graphics5/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_buffer_destroy(kope_g5_buffer *buffer);
void *kope_d3d12_buffer_lock(kope_g5_buffer *buffer);
void kope_d3d12_buffer_unlock(kope_g5_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
