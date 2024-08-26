#ifndef KOPE_D3D12_DEVICE_FUNCTIONS_HEADER
#define KOPE_D3D12_DEVICE_FUNCTIONS_HEADER

#include <kope/graphics5/device.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist);

void kope_d3d12_device_destroy(kope_g5_device *device);

void kope_d3d12_device_set_name(kope_g5_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif
