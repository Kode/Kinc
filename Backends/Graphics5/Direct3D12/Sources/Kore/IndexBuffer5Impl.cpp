#include "pch.h"

#include "Direct3D12.h"
#include "IndexBuffer5Impl.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/WinError.h>

#include <Windows.h>

using namespace Kore;

IndexBuffer5Impl* IndexBuffer5Impl::_current = nullptr;

IndexBuffer5Impl::IndexBuffer5Impl(int count, bool gpuMemory) : myCount(count), _gpuMemory(gpuMemory) {}

Graphics5::IndexBuffer::IndexBuffer(int count, bool gpuMemory) : IndexBuffer5Impl(count, gpuMemory) {
	static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	static const int uploadBufferSize = sizeof(int) * count;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&uploadBuffer));

	if (gpuMemory) {
		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_GRAPHICS_PPV_ARGS(&indexBuffer));

		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	}
	else {
		indexBufferView.BufferLocation = uploadBuffer->GetGPUVirtualAddress();
	}
	indexBufferView.SizeInBytes = uploadBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

Graphics5::IndexBuffer::~IndexBuffer() {
	// ib->Release();
	// delete[] indices;
}

int* Graphics5::IndexBuffer::lock() {
	void* p;
	uploadBuffer->Map(0, nullptr, &p);
	return (int*)p;
}

void Graphics5::IndexBuffer::unlock() {
	uploadBuffer->Unmap(0, nullptr);
}

void IndexBuffer5Impl::_upload(ID3D12GraphicsCommandList* commandList) {
	if (!_gpuMemory) return;

	commandList->CopyBufferRegion(indexBuffer, 0, uploadBuffer, 0, sizeof(int) * myCount);

	CD3DX12_RESOURCE_BARRIER barriers[1] = {
	    CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)};

	commandList->ResourceBarrier(1, barriers);
}

void Graphics5::IndexBuffer::_set() {
	_current = this;
}

int Graphics5::IndexBuffer::count() {
	return myCount;
}
