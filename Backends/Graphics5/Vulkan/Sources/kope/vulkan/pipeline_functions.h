#ifndef KOPE_VULKAN_PIPELINE_FUNCTIONS_HEADER
#define KOPE_VULKAN_PIPELINE_FUNCTIONS_HEADER

#include "device_structs.h"
#include "pipeline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_vulkan_render_pipeline_init(kope_vulkan_device *device, kope_vulkan_render_pipeline *pipe, const kope_vulkan_render_pipeline_parameters *parameters);

void kope_vulkan_render_pipeline_destroy(kope_vulkan_render_pipeline *pipe);

void kope_vulkan_compute_pipeline_init(kope_vulkan_device *device, kope_vulkan_compute_pipeline *pipe,
                                       const kope_vulkan_compute_pipeline_parameters *parameters);

void kope_vulkan_compute_pipeline_destroy(kope_vulkan_compute_pipeline *pipe);

void kope_vulkan_ray_pipeline_init(kope_g5_device *device, kope_vulkan_ray_pipeline *pipe, const kope_vulkan_ray_pipeline_parameters *parameters);

void kope_vulkan_ray_pipeline_destroy(kope_vulkan_ray_pipeline *pipe);

#ifdef __cplusplus
}
#endif

#endif
