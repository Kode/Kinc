#include "descriptorset_functions.h"
#include "descriptorset_structs.h"

#include <kope/util/align.h>

void kope_d3d12_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = buffer->d3d12.resource->GetGPUVirtualAddress();
	desc.SizeInBytes = align_pow2((int)buffer->d3d12.size, 256);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateConstantBufferView(&desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_buffer_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = 1;
	desc.Buffer.StructureByteStride = (UINT)buffer->d3d12.size;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(buffer->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_bvh_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.RaytracingAccelerationStructure.Location = buffer->d3d12.resource->GetGPUVirtualAddress();

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(nullptr, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = (DXGI_FORMAT)texture->d3d12.format;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(texture->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_view_uav(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_texture *texture, uint32_t index) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	desc.Format = (DXGI_FORMAT)texture->d3d12.format;
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateUnorderedAccessView(texture->d3d12.resource, NULL, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_sampler(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE src_handle = device->d3d12.all_samplers->GetCPUDescriptorHandleForHeapStart();
	src_handle.ptr += sampler->d3d12.sampler_index * device->d3d12.sampler_increment;

	D3D12_CPU_DESCRIPTOR_HANDLE dst_handle = device->d3d12.sampler_heap->GetCPUDescriptorHandleForHeapStart();
	dst_handle.ptr += (set->sampler_allocation.offset + index) * device->d3d12.sampler_increment;

	device->d3d12.device->CopyDescriptorsSimple(1, dst_handle, src_handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void kope_d3d12_descriptor_set_prepare_cbv_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer) {
	if (buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = buffer->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)buffer->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}
}

void kope_d3d12_descriptor_set_prepare_srv_texture(kope_g5_command_list *list, kope_g5_texture *texture) {
	if (texture->d3d12.resource_state != (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = texture->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)texture->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		texture->d3d12.resource_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
}

void kope_d3d12_descriptor_set_prepare_uav_texture(kope_g5_command_list *list, kope_g5_texture *texture) {
	if (texture->d3d12.resource_state != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = texture->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)texture->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		texture->d3d12.resource_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
}
