#ifndef KOPE_WEBGPU_DEVICE_FUNCTIONS_HEADER
#define KOPE_WEBGPU_DEVICE_FUNCTIONS_HEADER

#include <kope/graphics5/device.h>

#include <kinc/math/matrix.h>

#include "descriptorset_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_webgpu_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist);

void kope_webgpu_device_destroy(kope_g5_device *device);

void kope_webgpu_device_set_name(kope_g5_device *device, const char *name);

void kope_webgpu_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer);

void kope_webgpu_device_create_command_list(kope_g5_device *device, kope_g5_command_list_type type, kope_g5_command_list *list);

void kope_webgpu_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture);

void kope_webgpu_device_create_descriptor_set(kope_g5_device *device, uint32_t descriptor_count, uint32_t dynamic_descriptor_count,
                                              uint32_t bindless_descriptor_count, uint32_t sampler_count, kope_webgpu_descriptor_set *set);

void kope_webgpu_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler);

kope_g5_texture *kope_webgpu_device_get_framebuffer(kope_g5_device *device);

void kope_webgpu_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list);

void kope_webgpu_device_wait_until_idle(kope_g5_device *device);

void kope_webgpu_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count, kope_g5_buffer *index_buffer,
                                                 uint32_t index_count, kope_g5_raytracing_volume *volume);

void kope_webgpu_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                    uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy);

void kope_webgpu_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set);

void kope_webgpu_device_create_fence(kope_g5_device *device, kope_g5_fence *fence);

uint32_t kope_webgpu_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes);

void kope_webgpu_device_signal(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value);

void kope_webgpu_device_wait(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
