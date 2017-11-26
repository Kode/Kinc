#include "pch.h"

#include <Kore/Graphics5/ConstantBuffer.h>

#include "Direct3D12.h"
#include <Kore/WinError.h>

using namespace Kore;

Graphics5::ConstantBuffer::ConstantBuffer(int size) {
	mySize = size;
	data = nullptr;
	
	affirm(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&_buffer)));

	void* p;
	_buffer->Map(0, nullptr, &p);
	ZeroMemory(p, size);
	_buffer->Unmap(0, nullptr);
}

Graphics5::ConstantBuffer::~ConstantBuffer() {
	
}

void Graphics5::ConstantBuffer::lock() {
	lock(0, size());
}

void Graphics5::ConstantBuffer::lock(int start, int count) {
	lastStart = start;
	lastCount = count;
	D3D12_RANGE range;
	range.Begin = start;
	range.End = range.Begin + count;
	u8* p;
	_buffer->Map(0, &range, (void**)&p);
	data = &p[start];
}

void Graphics5::ConstantBuffer::unlock() {
	D3D12_RANGE range;
	range.Begin = lastStart;
	range.End = range.Begin + lastCount;
	_buffer->Unmap(0, &range);
	data = nullptr;
}

int Graphics5::ConstantBuffer::size() {
	return mySize;
}
