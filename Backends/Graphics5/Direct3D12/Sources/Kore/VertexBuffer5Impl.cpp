#include "pch.h"

#include "Direct3D12.h"
#include "VertexBuffer5Impl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/WinError.h>

using namespace Kore;

VertexBuffer5Impl* VertexBuffer5Impl::_current = nullptr;

VertexBuffer5Impl::VertexBuffer5Impl(int count) : myCount(count), lastStart(-1), lastCount(-1) {}

Graphics5::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, bool gpuMemory, int instanceDataStepRate) : VertexBuffer5Impl(count) {
	static_assert(sizeof(D3D12VertexBufferView) == sizeof(D3D12_VERTEX_BUFFER_VIEW), "Something is wrong with D3D12IVertexBufferView");

	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		switch (structure.elements[i].data) {
		case Graphics4::Float1VertexData:
			myStride += 1 * 4;
			break;
		case Graphics4::Float2VertexData:
			myStride += 2 * 4;
			break;
		case Graphics4::Float3VertexData:
			myStride += 3 * 4;
			break;
		case Graphics4::Float4VertexData:
			myStride += 4 * 4;
			break;
		case Graphics4::ColorVertexData:
			myStride += 1 * 4;
			break;
		}
	}

	int uploadBufferSize = myStride * myCount;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
									IID_GRAPHICS_PPV_ARGS(&uploadBuffer));

	// device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	// &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	//	D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer));

	view.BufferLocation = uploadBuffer->GetGPUVirtualAddress();
	view.SizeInBytes = uploadBufferSize;
	view.StrideInBytes = myStride;
}

Graphics5::VertexBuffer::~VertexBuffer() {
	// vb->Release();
	// delete[] vertices;
}

float* Graphics5::VertexBuffer::lock() {
	return lock(0, count());
}

float* Graphics5::VertexBuffer::lock(int start, int count) {
	lastStart = start;
	lastCount = count;
	void* p;
	D3D12_RANGE range;
	range.Begin = start * myStride;
	range.End = range.Begin + count * myStride;
	uploadBuffer->Map(0, &range, &p);
	byte* bytes = (byte*)p;
	bytes += start * myStride;
	return (float*)bytes;
}

void Graphics5::VertexBuffer::unlock() {
	D3D12_RANGE range;
	range.Begin = lastStart * myStride;
	range.End = range.Begin + lastCount * myStride;
	uploadBuffer->Unmap(0, &range);

	//view.BufferLocation = uploadBuffer->GetGPUVirtualAddress() + myStart * myStride;

	// commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, count() * stride());
	// CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
	// D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
	// commandList->ResourceBarrier(1, barriers);
}

int Graphics5::VertexBuffer::_set(int offset) {
	// UINT stride = myStride;
	// UINT offset = 0;
	// context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	_current = this;
	return 0;
}

int Graphics5::VertexBuffer::count() {
	return myCount;
}

int Graphics5::VertexBuffer::stride() {
	return myStride;
}
