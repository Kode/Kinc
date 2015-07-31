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
	
	//indices = new int[count];

	affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * 4),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ib)));

	/*affirm(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * 4),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ibUpload)));

	ib->SetName(L"Index Buffer Resource");
	ibUpload->SetName(L"Index Buffer Upload Resource");*/

	view.BufferLocation = ib->GetGPUVirtualAddress();
	view.SizeInBytes = myCount * 4;
	view.Format = DXGI_FORMAT_R32_SINT;
}

IndexBuffer::~IndexBuffer() {
	ib->Release();
	//delete[] indices;
}

int* IndexBuffer::lock() {
	//return indices;
	int* data;
	affirm(ib->Map(0, nullptr, reinterpret_cast<void**>(&data)));
	return data;
}

void IndexBuffer::unlock() {
	//context->UpdateSubresource(ib, 0, nullptr, indices, 0, 0);

	/*D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = (void*)indices;
	indexData.RowPitch = myCount * 4;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(commandList, ib, ibUpload, 0, 0, 1, &indexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ib, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));*/

	ib->Unmap(0, nullptr);
}

void IndexBuffer::set() {
	_current = this;
	//context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);


}

int IndexBuffer::count() {
	return myCount;
}
