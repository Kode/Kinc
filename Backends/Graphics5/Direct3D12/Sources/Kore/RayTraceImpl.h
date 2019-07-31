#pragma once

#ifdef KORE_DXR

#include <d3d12.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ID3D12StateObject* dxr_state;
	ID3D12Resource* raygen_shader_table;
	ID3D12Resource* miss_shader_table;
	ID3D12Resource* hitgroup_shader_table;
} kinc_raytrace_pipeline_impl_t;

typedef struct {
	ID3D12Resource* bottom_level_accel;
	ID3D12Resource* top_level_accel;
} kinc_raytrace_acceleration_structure_impl_t;

typedef struct {
	ID3D12Resource* _texture;
	D3D12_GPU_DESCRIPTOR_HANDLE _texture_handle;
} kinc_raytrace_target_impl_t;

#ifdef __cplusplus
}
#endif

#endif
