#include "pch.h"

#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/PipelineState.h>

#include "Direct3D12.h"

using namespace Kore;
using namespace Kore::Graphics5;

extern ID3D12CommandQueue* commandQueue;

namespace {

	int currentInstance = 0;

	ID3D12Resource* vertexConstantBuffers[QUEUE_SLOT_COUNT * 128];
	ID3D12Resource* fragmentConstantBuffers[QUEUE_SLOT_COUNT * 128];
	bool created = false;

	void createConstantBuffer() {
		if (created) return;
		created = true;

		for (int i = 0; i < QUEUE_SLOT_COUNT * 128; ++i) {
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexConstants)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&vertexConstantBuffers[i]));

			void* p;
			vertexConstantBuffers[i]->Map(0, nullptr, &p);
			ZeroMemory(p, sizeof(vertexConstants));
			vertexConstantBuffers[i]->Unmap(0, nullptr);

			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(fragmentConstants)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&fragmentConstantBuffers[i]));

			fragmentConstantBuffers[i]->Map(0, nullptr, &p);
			ZeroMemory(p, sizeof(fragmentConstants));
			fragmentConstantBuffers[i]->Unmap(0, nullptr);
		}
	}
}

CommandList::CommandList() {
	closed = false;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&_commandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_GRAPHICS_PPV_ARGS(&_commandList));
	//_commandList->Close();
	createConstantBuffer();
	_indexCount = 0;
}

CommandList::~CommandList() {

}

void CommandList::begin() {
	if (closed) {
		_commandAllocator->Reset();
		_commandList->Reset(_commandAllocator, nullptr);
	}
}

void CommandList::end() {
	_commandList->Close();
	closed = true;

	ID3D12CommandList* commandLists[] = { _commandList };
	commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
}

void CommandList::clear(RenderTarget* renderTarget, uint flags, uint color, float depth, int stencil) {
	float clearColor[] = { ((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		((color & 0xff000000) >> 24) / 255.0f };
	if (flags & ClearColorFlag) {
		_commandList->ClearRenderTargetView(renderTarget->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}
	if ((flags & ClearDepthFlag) || (flags & ClearStencilFlag)) {
		D3D12_CLEAR_FLAGS flags =
			(flags & ClearDepthFlag) && (flags & ClearStencilFlag)
			? D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL
			: (flags & ClearDepthFlag) ? D3D12_CLEAR_FLAG_DEPTH : D3D12_CLEAR_FLAG_STENCIL;
		_commandList->ClearDepthStencilView(renderTarget->depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), flags, depth, stencil, 0, nullptr);
	}
	if ((flags & ClearDepthFlag) || (flags & ClearStencilFlag)) {
		_commandList->ClearDepthStencilView(renderTarget->depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}
}

void CommandList::renderTargetToFramebufferBarrier(RenderTarget* renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	_commandList->ResourceBarrier(1, &barrier);
}

void CommandList::framebufferToRenderTargetBarrier(RenderTarget* renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	_commandList->ResourceBarrier(1, &barrier);
}

void CommandList::drawIndexedVertices() {
	drawIndexedVertices(0, _indexCount);
}

void CommandList::drawIndexedVertices(int start, int count) {
	PipelineState5Impl::setConstants(_commandList, _currentPipeline);

	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	u8* data;
	vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->Map(0, nullptr, (void**)&data);
	memcpy(data, vertexConstants, sizeof(vertexConstants));
	vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->Unmap(0, nullptr);

	fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->Map(0, nullptr, (void**)&data);
	memcpy(data, fragmentConstants, sizeof(fragmentConstants));
	fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->Unmap(0, nullptr);

	_commandList->SetGraphicsRootConstantBufferView(1, vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->GetGPUVirtualAddress());
	_commandList->SetGraphicsRootConstantBufferView(2, fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->GetGPUVirtualAddress());
	if (++currentInstance >= 128) currentInstance = 0;

	_commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);
}

void CommandList::viewport(int x, int y, int width, int height) {
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = static_cast<float>(x);
	viewport.TopLeftY = static_cast<float>(y);
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	_commandList->RSSetViewports(1, &viewport);
}

void CommandList::scissor(int x, int y, int width, int height) {
	D3D12_RECT scissor;
	scissor.left = x;
	scissor.top = y;
	scissor.right = x + width;
	scissor.bottom = y + height;
	_commandList->RSSetScissorRects(1, &scissor);
}

void CommandList::disableScissor() {
	
}

void CommandList::setPipeline(PipelineState* pipeline) {
	_currentPipeline = pipeline;
	_commandList->SetPipelineState(pipeline->pso);
}

void CommandList::setVertexBuffers(VertexBuffer** buffers, int count) {
	_commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&buffers[0]->view);
}

void CommandList::setIndexBuffer(IndexBuffer& buffer) {
	_indexCount = buffer.count();
	_commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&buffer.indexBufferView);
}

//void CommandList::restoreRenderTarget() {
	//_commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, &depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//_commandList->RSSetViewports(1, &::viewport);
	//_commandList->RSSetScissorRects(1, &rectScissor);
//}

void CommandList::setRenderTargets(RenderTarget** targets, int count) {
	_commandList->OMSetRenderTargets(1, &targets[0]->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true,
		&targets[0]->depthStencilDescriptorHeap != nullptr ? &targets[0]->depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart() : nullptr);
	_commandList->RSSetViewports(1, (D3D12_VIEWPORT*)&targets[0]->viewport);
	_commandList->RSSetScissorRects(1, (D3D12_RECT*)&targets[0]->scissor);
}

void CommandList::upload(IndexBuffer* buffer) {
	buffer->_upload(_commandList);
}

void CommandList::upload(Texture* texture) {
	D3D12_SUBRESOURCE_DATA srcData;
	srcData.pData = texture->data;
	srcData.RowPitch = texture->format == Image::RGBA32 ? (texture->width * 4) : texture->width;
	srcData.SlicePitch = texture->format == Image::RGBA32 ? (texture->width * texture->height * 4) : (texture->width * texture->height);
	UpdateSubresources(_commandList, texture->image, texture->uploadImage, 0, 0, 1, &srcData);
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
