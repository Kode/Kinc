#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include "Direct3D11.h"
#include "IndexBufferImpl.h"
#include <Kore/WinError.h>
#include <Windows.h>
#include <d3d11.h>

using namespace Kore;

IndexBuffer* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {
	
}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	indices = new int[count];

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth           = sizeof(unsigned int) * count;
	bufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags      = 0;
	bufferDesc.MiscFlags           = 0;
	bufferDesc.StructureByteStride = 0;

	affirm(device->CreateBuffer(&bufferDesc, nullptr, &ib));
}

IndexBuffer::~IndexBuffer() {
	ib->Release();
	delete[] indices;
}

int* IndexBuffer::lock() {
	return indices;
}

void IndexBuffer::unlock() {
	context->UpdateSubresource(ib, 0, nullptr, indices, 0, 0);
}

void IndexBuffer::_set() {
	_current = this;
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
}

int IndexBuffer::count() {
	return myCount;
}
