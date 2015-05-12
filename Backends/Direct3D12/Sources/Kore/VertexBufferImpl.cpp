#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>
#include "Direct3D12.h"
#include "d3dx12.h"

using namespace Kore;

VertexBufferImpl::VertexBufferImpl(int count) : myCount(count) {
	
}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure) : VertexBufferImpl(count) {
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

	vertices = new float[myStride / 4 * myCount];

	affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(myStride * myCount),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vb)));

	affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(myStride * myCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vbUpload)));

	vb->SetName(L"Vertex Buffer Resource");
	vbUpload->SetName(L"Vertex Buffer Upload Resource");
}

VertexBuffer::~VertexBuffer() {
	vb->Release();
	delete[] vertices;
}

float* VertexBuffer::lock() {
	return lock(0, count());
}

float* VertexBuffer::lock(int start, int count) {
	return &vertices[start * myStride / 4];
}

void VertexBuffer::unlock() {
	//context->UpdateSubresource(vb, 0, nullptr, vertices, 0, 0);

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = (void*)vertices;
	vertexData.RowPitch = myStride * myCount;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(commandList, vb, vbUpload, 0, 0, 1, &vertexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vb, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void VertexBuffer::set() {
	UINT stride = myStride;
	UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);


}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}
