#include "indexbuffer.h"

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/graphics5/indexbuffer.h>

kinc_g5_index_buffer_t *_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.myCount = count;
	buffer->impl._gpuMemory = gpuMemory;
	// static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	int uploadBufferSize = sizeof(int) * count;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_GRAPHICS_PPV_ARGS(&buffer->impl.uploadBuffer)));

	if (gpuMemory) {
		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		                                                              D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_GRAPHICS_PPV_ARGS(&buffer->impl.indexBuffer)));

		buffer->impl.indexBufferView.BufferLocation = buffer->impl.indexBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.indexBuffer);
	}
	else {
		buffer->impl.indexBufferView.BufferLocation = buffer->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.uploadBuffer);
	}
	buffer->impl.indexBufferView.SizeInBytes = uploadBufferSize;
	buffer->impl.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	buffer->impl.indexBuffer->lpVtbl->Release(buffer->impl.indexBuffer);
	buffer->impl.uploadBuffer->lpVtbl->Release(buffer->impl.uploadBuffer);
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	void *p;
	buffer->impl.uploadBuffer->lpVtbl->Map(buffer->impl.uploadBuffer, 0, NULL, &p);
	return (int *)p;
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	buffer->impl.uploadBuffer->lpVtbl->Unmap(buffer->impl.uploadBuffer, 0, NULL);
}

void kinc_g5_internal_index_buffer_upload(kinc_g5_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl._gpuMemory) return;

	commandList->lpVtbl->CopyBufferRegion(commandList, buffer->impl.indexBuffer, 0, buffer->impl.uploadBuffer, 0, sizeof(int) * buffer->impl.myCount);

	CD3DX12_RESOURCE_BARRIER barriers[1] = {
	    CD3DX12_RESOURCE_BARRIER::Transition(buffer->impl.indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)};

	commandList->lpVtbl->ResourceBarrier(commandList, 1, barriers);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {
	_current_index_buffer = buffer;
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
