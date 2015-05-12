#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include "Direct3D12.h"
#include "IndexBufferImpl.h"
#include <Kore/WinError.h>
#include <Windows.h>
#include "d3dx12.h"

using namespace Kore;

IndexBuffer* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {
	
}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	indices = new int[count];

	affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * 4),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&ib)));

	affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * 4),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ibUpload)));

	ib->SetName(L"Index Buffer Resource");
	ibUpload->SetName(L"Index Buffer Upload Resource");
}

IndexBuffer::~IndexBuffer() {
	ib->Release();
	delete[] indices;
}

int* IndexBuffer::lock() {
	return indices;
}

void IndexBuffer::unlock() {
	//context->UpdateSubresource(ib, 0, nullptr, indices, 0, 0);

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = (void*)indices;
	indexData.RowPitch = myCount * 4;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(commandList, ib, ibUpload, 0, 0, 1, &indexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ib, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void IndexBuffer::set() {
	_current = this;
	//context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);


}

int IndexBuffer::count() {
	return myCount;
}
