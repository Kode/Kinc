#ifndef KOPE_D3D12_DESCRIPTORSET_FUNCTIONS_HEADER
#define KOPE_D3D12_DESCRIPTORSET_FUNCTIONS_HEADER

#include "buffer_structs.h"
#include "descriptorset_structs.h"
#include "device_structs.h"

#include <kope/graphics5/sampler.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index);
void kope_d3d12_descriptor_set_set_buffer_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index);
void kope_d3d12_descriptor_set_set_bvh_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_raytracing_hierarchy *bvh, uint32_t index);
void kope_d3d12_descriptor_set_set_texture_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture,
                                                    uint32_t highest_mip_level, uint32_t mip_count, uint32_t index);
void kope_d3d12_descriptor_set_set_texture_array_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t index);
void kope_d3d12_descriptor_set_set_texture_cube_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t index);
void kope_d3d12_descriptor_set_set_texture_view_uav(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t mip_level,
                                                    uint32_t index);
void kope_d3d12_descriptor_set_set_sampler(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index);

void kope_d3d12_descriptor_set_prepare_cbv_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer);
void kope_d3d12_descriptor_set_prepare_srv_texture(kope_g5_command_list *list, kope_g5_texture *texture, uint32_t highest_mip_level, uint32_t mip_count);
void kope_d3d12_descriptor_set_prepare_uav_texture(kope_g5_command_list *list, kope_g5_texture *texture, uint32_t mip_level);

#ifdef __cplusplus
}
#endif

#endif
