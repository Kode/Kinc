#include "pch.h"

#include "Direct3D11.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/WinError.h>

using namespace Kore;

VertexBufferImpl::VertexBufferImpl(int count) : myCount(count) {}

Graphics4::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count) {
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
		case Float4x4VertexData:
			myStride += 4 * 4 * 4;
			break;
		}
	}

	vertices = new float[myStride / 4 * myCount];

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = myStride * myCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	affirm(device->CreateBuffer(&bufferDesc, nullptr, &_vb));
}

Graphics4::VertexBuffer::~VertexBuffer() {
	_vb->Release();
	delete[] vertices;
}

float* Graphics4::VertexBuffer::lock() {
	return lock(0, count());
}

float* Graphics4::VertexBuffer::lock(int start, int count) {
	return &vertices[start * myStride / 4];
}

void Graphics4::VertexBuffer::unlock() {

	context->UpdateSubresource(_vb, 0, nullptr, vertices, 0, 0);
}

int Graphics4::VertexBuffer::_set(int offset) {
	// UINT stride = myStride;
	// UINT internaloffset = 0;
	// context->IASetVertexBuffers(0, 1, &vb, &stride, &internaloffset);
	return 0;
}

int Graphics4::VertexBuffer::count() {
	return myCount;
}

int Graphics4::VertexBuffer::stride() {
	return myStride;
}
