#include "indexbuffer.h"

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/graphics5/indexbuffer.h>

kinc_g5_index_buffer_t *_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, kinc_g5_index_buffer_format_t format, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.gpu_memory = gpuMemory;
	buffer->impl.format = format;

	// static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	int uploadBufferSize = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * count : sizeof(uint32_t) * count;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = uploadBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &buffer->impl.upload_buffer));

	if (gpuMemory) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		                                                              D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &buffer->impl.index_buffer));

		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.index_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.index_buffer);
	}
	else {
		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.upload_buffer);
	}
	buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	buffer->impl.index_buffer_view.Format = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	buffer->impl.index_buffer->lpVtbl->Release(buffer->impl.index_buffer);
	buffer->impl.upload_buffer->lpVtbl->Release(buffer->impl.upload_buffer);
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	void *p;
	buffer->impl.upload_buffer->lpVtbl->Map(buffer->impl.upload_buffer, 0, NULL, &p);
	return (int *)p;
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	buffer->impl.upload_buffer->lpVtbl->Unmap(buffer->impl.upload_buffer, 0, NULL);
}

void kinc_g5_internal_index_buffer_upload(kinc_g5_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl.gpu_memory) return;

	commandList->lpVtbl->CopyBufferRegion(commandList, buffer->impl.index_buffer, 0, buffer->impl.upload_buffer, 0,
	                                      buffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * buffer->impl.count
	                                                                                               : sizeof(uint32_t) * buffer->impl.count);

	D3D12_RESOURCE_BARRIER barriers[1] = {0};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[0].Transition.pResource = buffer->impl.index_buffer;
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->lpVtbl->ResourceBarrier(commandList, 1, barriers);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {
	_current_index_buffer = buffer;
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
