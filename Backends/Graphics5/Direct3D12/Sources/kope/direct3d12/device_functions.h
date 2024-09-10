#ifndef KOPE_D3D12_DEVICE_FUNCTIONS_HEADER
#define KOPE_D3D12_DEVICE_FUNCTIONS_HEADER

#include <kope/graphics5/device.h>

#include "descriptorset_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist);

void kope_d3d12_device_destroy(kope_g5_device *device);

void kope_d3d12_device_set_name(kope_g5_device *device, const char *name);

void kope_d3d12_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer);

void kope_d3d12_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list);

void kope_d3d12_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture);

void kope_d3d12_device_create_descriptor_set(kope_g5_device *device, uint32_t descriptor_count, uint32_t sampler_count, kope_d3d12_descriptor_set *set);

kope_g5_texture *kope_d3d12_device_get_framebuffer(kope_g5_device *device);

void kope_d3d12_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list);

#ifdef __cplusplus
}
#endif

#endif
