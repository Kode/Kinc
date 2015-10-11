#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include "Direct3D12.h"
#include "IndexBufferImpl.h"
#include <Kore/WinError.h>
#include <Windows.h>
#include "d3dx12.h"

using namespace Kore;

IndexBufferImpl* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {
	
}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	static const int uploadBufferSize = sizeof(int) * count;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&indexBuffer));

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = uploadBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

IndexBuffer::~IndexBuffer() {
	//ib->Release();
	//delete[] indices;
}

int* IndexBuffer::lock() {
	void* p;
	uploadBuffer->Map(0, nullptr, &p);
	return (int*)p;
}

void IndexBuffer::unlock() {
	uploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(indexBuffer, 0, uploadBuffer, 0, sizeof(int) * count());

	CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER) };

	commandList->ResourceBarrier(1, barriers);
}

void IndexBuffer::_set() {
	_current = this;
}

int IndexBuffer::count() {
	return myCount;
}
