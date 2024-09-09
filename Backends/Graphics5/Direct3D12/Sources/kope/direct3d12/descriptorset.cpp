#include "descriptorset_functions.h"
#include "descriptorset_structs.h"

void kope_d3d12_descriptor_set_set_buffer_view_cbv(kope_g5_device *device, kope_d3d12_descriptor_set *set, kope_g5_buffer *buffer, uint32_t index) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	desc.BufferLocation = buffer->d3d12.resource->GetGPUVirtualAddress();
	desc.SizeInBytes = (UINT)buffer->d3d12.size;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += (set->allocation.offset + index) * device->d3d12.cbv_srv_uav_increment;
	device->d3d12.device->CreateConstantBufferView(&desc, descriptor_handle);
}
