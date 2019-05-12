#include "pch.h"

#include "Direct3D12.h"
#include "IndexBuffer5Impl.h"

#include <Kinc/Graphics5/IndexBuffer.h>
#include <Kore/SystemMicrosoft.h>

#include <Windows.h>

#include <assert.h>

kinc_g5_index_buffer_t *_current_index_buffer = nullptr;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.myCount = count;
	buffer->impl._gpuMemory = gpuMemory;
	static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	int uploadBufferSize = sizeof(int) * count;

	assert(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&buffer->impl.uploadBuffer)) == S_OK);

	if (gpuMemory) {
		assert(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		                                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		                                IID_GRAPHICS_PPV_ARGS(&buffer->impl.indexBuffer)) == S_OK);

		buffer->impl.indexBufferView.BufferLocation = buffer->impl.indexBuffer->GetGPUVirtualAddress();
	}
	else {
		buffer->impl.indexBufferView.BufferLocation = buffer->impl.uploadBuffer->GetGPUVirtualAddress();
	}
	buffer->impl.indexBufferView.SizeInBytes = uploadBufferSize;
	buffer->impl.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	// ib->Release();
	// delete[] indices;
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	void* p;
	buffer->impl.uploadBuffer->Map(0, nullptr, &p);
	return (int*)p;
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	buffer->impl.uploadBuffer->Unmap(0, nullptr);
}

void kinc_g5_internal_index_buffer_upload(kinc_g5_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl._gpuMemory) return;

	commandList->CopyBufferRegion(buffer->impl.indexBuffer, 0, buffer->impl.uploadBuffer, 0, sizeof(int) * buffer->impl.myCount);

	CD3DX12_RESOURCE_BARRIER barriers[1] = {
	    CD3DX12_RESOURCE_BARRIER::Transition(buffer->impl.indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)};

	commandList->ResourceBarrier(1, barriers);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {
	_current_index_buffer = buffer;
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
