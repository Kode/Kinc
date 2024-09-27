#include "descriptorset_functions.h"
#include "descriptorset_structs.h"

#include <kope/direct3d12/texture_functions.h>

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

void kope_d3d12_descriptor_set_set_bvh_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_raytracing_hierarchy *bvh, uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.RaytracingAccelerationStructure.Location = bvh->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(nullptr, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                    uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	DXGI_FORMAT format = (DXGI_FORMAT)texture_view->texture->d3d12.format;
	switch (format) {
	case DXGI_FORMAT_D16_UNORM:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	default:
		desc.Format = format;
		break;
	}

	desc.Texture2D.MipLevels = texture_view->mip_level_count;
	desc.Texture2D.MostDetailedMip = texture_view->base_mip_level;
	desc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(texture_view->texture->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_array_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                          uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	DXGI_FORMAT format = (DXGI_FORMAT)texture_view->texture->d3d12.format;
	switch (format) {
	case DXGI_FORMAT_D16_UNORM:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;
	default:
		desc.Format = format;
		break;
	}

	desc.Texture2DArray.MipLevels = texture_view->mip_level_count;
	desc.Texture2DArray.MostDetailedMip = texture_view->base_mip_level;
	desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	desc.Texture2DArray.FirstArraySlice = texture_view->base_array_layer;
	desc.Texture2DArray.ArraySize = texture_view->array_layer_count;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(texture_view->texture->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_cube_view_srv(kope_g5_device *device, kope_d3d12_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                         uint32_t index) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	DXGI_FORMAT format = (DXGI_FORMAT)texture_view->texture->d3d12.format;
	switch (format) {
	case DXGI_FORMAT_D16_UNORM:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;
	default:
		desc.Format = format;
		break;
	}

	desc.TextureCube.MipLevels = texture_view->mip_level_count;
	desc.TextureCube.MostDetailedMip = texture_view->base_mip_level;
	desc.TextureCube.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateShaderResourceView(texture_view->texture->d3d12.resource, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_texture_view_uav(kope_g5_device *device, kope_d3d12_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                    uint32_t index) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	desc.Format = (DXGI_FORMAT)texture_view->texture->d3d12.format;
	desc.Texture2D.MipSlice = texture_view->base_mip_level;
	desc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->descriptor_allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateUnorderedAccessView(texture_view->texture->d3d12.resource, NULL, &desc, descriptor_handle);
}

void kope_d3d12_descriptor_set_set_sampler(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE src_handle = device->d3d12.all_samplers->GetCPUDescriptorHandleForHeapStart();
	src_handle.ptr += sampler->d3d12.sampler_index * device->d3d12.sampler_increment;

	D3D12_CPU_DESCRIPTOR_HANDLE dst_handle = device->d3d12.sampler_heap->GetCPUDescriptorHandleForHeapStart();
	dst_handle.ptr += (set->sampler_allocation.offset + index) * device->d3d12.sampler_increment;

	device->d3d12.device->CopyDescriptorsSimple(1, dst_handle, src_handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void kope_d3d12_descriptor_set_prepare_cbv_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer) {
	if (buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER && buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_GENERIC_READ) {
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

void kope_d3d12_descriptor_set_prepare_srv_texture(kope_g5_command_list *list, const kope_g5_texture_view *texture_view) {
	for (uint32_t array_layer = texture_view->base_array_layer; array_layer < texture_view->base_array_layer + texture_view->array_layer_count; ++array_layer) {
		for (uint32_t mip_level = texture_view->base_mip_level; mip_level < texture_view->base_mip_level + texture_view->mip_level_count; ++mip_level) {
			if (texture_view->texture->d3d12.resource_states[mip_level] !=
			    (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) {
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Transition.pResource = texture_view->texture->d3d12.resource;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.StateBefore =
				    (D3D12_RESOURCE_STATES)
				        texture_view->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(texture_view->texture, mip_level, array_layer)];
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource =
				    D3D12CalcSubresource(mip_level, 0, 0, texture_view->texture->d3d12.mip_level_count, texture_view->texture->d3d12.depth_or_array_layers);

				list->d3d12.list->ResourceBarrier(1, &barrier);

				texture_view->texture->d3d12.resource_states[mip_level] =
				    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}
		}
	}
}

void kope_d3d12_descriptor_set_prepare_uav_texture(kope_g5_command_list *list, const kope_g5_texture_view *texture_view) {
	if (texture_view->texture->d3d12.resource_states[texture_view->base_mip_level] != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = texture_view->texture->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore =
		    (D3D12_RESOURCE_STATES)
		        texture_view->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(texture_view->texture, texture_view->base_mip_level, 0)];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12CalcSubresource(texture_view->base_mip_level, 0, 0, texture_view->texture->d3d12.mip_level_count,
		                                                      texture_view->texture->d3d12.depth_or_array_layers);

		list->d3d12.list->ResourceBarrier(1, &barrier);

		texture_view->texture->d3d12.resource_states[texture_view->base_mip_level] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
}
