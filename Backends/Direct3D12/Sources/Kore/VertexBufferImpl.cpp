#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>
#include "Direct3D12.h"
#include "d3dx12.h"

using namespace Kore;

namespace {
	const int multiple = 100;
}

VertexBufferImpl* VertexBufferImpl::_current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count) : myCount(count), currentIndex(0) {
	
}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count) {
	static_assert(sizeof(D3D12VertexBufferView) == sizeof(D3D12_VERTEX_BUFFER_VIEW), "Something is wrong with D3D12IVertexBufferView");

	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		switch (structure.elements[i].data) {
		case Float1VertexData:
			myStride += 1 * 4;
			break;
		case Float2VertexData:
			myStride += 2 * 4;
			break;
		case Float3VertexData:
			myStride += 3 * 4;
			break;
		case Float4VertexData:
			myStride += 4 * 4;
			break;
		case ColorVertexData:
			myStride += 1 * 4;
			break;
		}
	}

	static const int uploadBufferSize = myStride * myCount;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize * multiple),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	//device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	//	D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer));

	view.BufferLocation = uploadBuffer->GetGPUVirtualAddress();
	view.SizeInBytes = uploadBufferSize;
	view.StrideInBytes = myStride;
}

VertexBuffer::~VertexBuffer() {
	//vb->Release();
	//delete[] vertices;
}

float* VertexBuffer::lock() {
	return lock(0, count());
}

float* VertexBuffer::lock(int start, int count) {
	void* p;
	D3D12_RANGE range;
	range.Begin = currentIndex * myCount * myStride;
	range.End = range.Begin + myCount * myStride;
	uploadBuffer->Map(0, &range, &p);
	byte* bytes = (byte*)p;
	bytes += currentIndex * myCount * myStride;
	return (float*)bytes;
}

void VertexBuffer::unlock() {
	D3D12_RANGE range;
	range.Begin = currentIndex * myCount * myStride;
	range.End = range.Begin + myCount * myStride;
	uploadBuffer->Unmap(0, &range);

	view.BufferLocation = uploadBuffer->GetGPUVirtualAddress() + currentIndex * myCount * myStride;
	
	++currentIndex;
	if (currentIndex >= multiple) {
		currentIndex = 0;
	}
	
	//commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, count() * stride());
	//CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
	//commandList->ResourceBarrier(1, barriers);
}

int VertexBuffer::_set(int offset) {
	//UINT stride = myStride;
	//UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	_current = this;
	return 0;
}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}
