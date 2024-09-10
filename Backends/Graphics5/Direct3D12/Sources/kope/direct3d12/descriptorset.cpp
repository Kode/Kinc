#include "descriptorset_functions.h"
#include "descriptorset_structs.h"

void kope_d3d12_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = buffer->d3d12.resource->GetGPUVirtualAddress();
	desc.SizeInBytes = (UINT)buffer->d3d12.size;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateConstantBufferView(&desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(texture->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_sampler(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE src_handle = device->d3d12.all_samplers->GetCPUDescriptorHandleForHeapStart();
	src_handle.ptr += sampler->d3d12.sampler_index * device->d3d12.sampler_increment;

	D3D12_CPU_DESCRIPTOR_HANDLE dst_handle = device->d3d12.sampler_heap->GetCPUDescriptorHandleForHeapStart();
	dst_handle.ptr += (set->sampler_allocation.offset + index) * device->d3d12.sampler_increment;

	device->d3d12.device->CopyDescriptorsSimple(1, dst_handle, src_handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}
