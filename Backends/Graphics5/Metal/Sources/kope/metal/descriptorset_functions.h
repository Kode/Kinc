#ifndef KOPE_METAL_DESCRIPTORSET_FUNCTIONS_HEADER
#define KOPE_METAL_DESCRIPTORSET_FUNCTIONS_HEADER

#include "buffer_structs.h"
#include "descriptorset_structs.h"
#include "device_structs.h"

#include <kope/graphics5/sampler.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_metal_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_metal_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index);
void kope_metal_descriptor_set_set_buffer_view_srv(kope_g5_device *device, kope_metal_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index);
void kope_metal_descriptor_set_set_bvh_view_srv(kope_g5_device *device, kope_metal_descriptor_set *set, kope_g5_raytracing_hierarchy *bvh, uint32_t index);
void kope_metal_descriptor_set_set_texture_view_srv(kope_g5_device *device, uint32_t offset, const kope_g5_texture_view *texture_view);
void kope_metal_descriptor_set_set_texture_array_view_srv(kope_g5_device *device, kope_metal_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                          uint32_t index);
void kope_metal_descriptor_set_set_texture_cube_view_srv(kope_g5_device *device, kope_metal_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                         uint32_t index);
void kope_metal_descriptor_set_set_texture_view_uav(kope_g5_device *device, kope_metal_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                    uint32_t index);
void kope_metal_descriptor_set_set_sampler(kope_g5_device *device, kope_metal_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index);

void kope_metal_descriptor_set_prepare_cbv_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, uint32_t offset, uint32_t size);
void kope_metal_descriptor_set_prepare_srv_texture(kope_g5_command_list *list, const kope_g5_texture_view *texture_view);
void kope_metal_descriptor_set_prepare_uav_texture(kope_g5_command_list *list, const kope_g5_texture_view *texture_view);

#ifdef __cplusplus
}
#endif

#endif
