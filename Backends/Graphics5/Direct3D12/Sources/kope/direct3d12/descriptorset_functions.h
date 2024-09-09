#ifndef KOPE_D3D12_DESCRIPTORSET_FUNCTIONS_HEADER
#define KOPE_D3D12_DESCRIPTORSET_FUNCTIONS_HEADER

#include "buffer_structs.h"
#include "descriptorset_structs.h"
#include "device_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
