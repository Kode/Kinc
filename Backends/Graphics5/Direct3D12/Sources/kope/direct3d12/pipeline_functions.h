#ifndef KOPE_D3D12_PIPELINE_FUNCTIONS_HEADER
#define KOPE_D3D12_PIPELINE_FUNCTIONS_HEADER

#include "device_structs.h"
#include "pipeline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_render_pipeline_init(kope_d3d12_device *device, kope_d3d12_pipeline *pipe, const kope_d3d12_render_pipeline_parameters *parameters);

void kope_d3d12_render_pipeline_destroy(kope_d3d12_pipeline *pipe);

void kope_d3d12_compute_pipeline_init(kope_d3d12_device *device, kope_d3d12_pipeline *pipe, const kope_d3d12_compute_pipeline_parameters *parameters);

void kope_d3d12_compute_pipeline_destroy(kope_d3d12_pipeline *pipe);

#ifdef __cplusplus
}
#endif

#endif
