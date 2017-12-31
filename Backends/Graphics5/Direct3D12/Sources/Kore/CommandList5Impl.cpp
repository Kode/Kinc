#include "pch.h"

#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/Graphics5/ConstantBuffer.h>

#include "Direct3D12.h"

using namespace Kore;
using namespace Kore::Graphics5;

extern ID3D12CommandQueue* commandQueue;

namespace {
	/*const int constantBufferMultiply = 1024;
	int currentConstantBuffer = 0;
	ID3D12Resource* vertexConstantBuffer;
	ID3D12Resource* fragmentConstantBuffer;
	bool created = false;

	void createConstantBuffer() {
		if (created) return;
		created = true;

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexConstants) * constantBufferMultiply),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&vertexConstantBuffer));

		void* p;
		vertexConstantBuffer->Map(0, nullptr, &p);
		ZeroMemory(p, sizeof(vertexConstants) * constantBufferMultiply);
		vertexConstantBuffer->Unmap(0, nullptr);

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(fragmentConstants) * constantBufferMultiply),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&fragmentConstantBuffer));

		fragmentConstantBuffer->Map(0, nullptr, &p);
		ZeroMemory(p, sizeof(fragmentConstants) * constantBufferMultiply);
		fragmentConstantBuffer->Unmap(0, nullptr);
	}*/

	UINT64 renderFenceValue = 0;
	ID3D12Fence* renderFence;
	HANDLE renderFenceEvent;

	void init() {
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			renderFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&renderFence));
		}
	}

	void waitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void graphicsFlushAndWait(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* commandAllocator, RenderTarget* renderTarget) {
		commandList->Close();

		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);

		commandQueue->Signal(renderFence, ++renderFenceValue);

		waitForFence(renderFence, renderFenceValue, renderFenceEvent);
		commandAllocator->Reset();
		commandList->Reset(commandAllocator, nullptr);
		commandList->OMSetRenderTargets(1, &renderTarget->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
		commandList->RSSetViewports(1, (D3D12_VIEWPORT*)&renderTarget->viewport);
		commandList->RSSetScissorRects(1, (D3D12_RECT*)&renderTarget->scissor);
	}

	RenderTarget* currentRenderTarget = nullptr;
}

CommandList::CommandList() {
	::init();
	closed = false;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&_commandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_GRAPHICS_PPV_ARGS(&_commandList));
	//_commandList->Close();
	//createConstantBuffer();
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
	int a = 3;
	++a;
	float clearColor[] = { ((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		((color & 0xff000000) >> 24) / 255.0f };
	if (flags & ClearColorFlag) {
		_commandList->ClearRenderTargetView(renderTarget->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}
	if ((flags & ClearDepthFlag) || (flags & ClearStencilFlag)) {
		D3D12_CLEAR_FLAGS d3dflags =
			(flags & ClearDepthFlag) && (flags & ClearStencilFlag)
			? D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL
			: (flags & ClearDepthFlag) ? D3D12_CLEAR_FLAG_DEPTH : D3D12_CLEAR_FLAG_STENCIL;
		_commandList->ClearDepthStencilView(renderTarget->depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), d3dflags, depth, stencil, 0, nullptr);
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

void CommandList::textureToRenderTargetBarrier(RenderTarget* renderTarget) {
	if (renderTarget->resourceState != RenderTargetResourceStateRenderTarget) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		_commandList->ResourceBarrier(1, &barrier);
		renderTarget->resourceState = RenderTargetResourceStateRenderTarget;
	}
}

void CommandList::renderTargetToTextureBarrier(RenderTarget* renderTarget) {
	if (renderTarget->resourceState != RenderTargetResourceStateTexture) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		_commandList->ResourceBarrier(1, &barrier);
		renderTarget->resourceState = RenderTargetResourceStateTexture;
	}
}

void CommandList::setPipelineLayout() {
	PipelineState5Impl::setConstants(_commandList, _currentPipeline);
}

void CommandList::setVertexConstantBuffer(ConstantBuffer* buffer, int offset) {
	_commandList->SetGraphicsRootConstantBufferView(1, buffer->_buffer->GetGPUVirtualAddress() + offset);
}

void CommandList::setFragmentConstantBuffer(ConstantBuffer* buffer, int offset) {
	_commandList->SetGraphicsRootConstantBufferView(2, buffer->_buffer->GetGPUVirtualAddress() + offset);
}

void CommandList::drawIndexedVertices() {
	drawIndexedVertices(0, _indexCount);
}

void CommandList::drawIndexedVertices(int start, int count) {
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	/*u8* data;
	D3D12_RANGE range;
	range.Begin = currentConstantBuffer * sizeof(vertexConstants);
	range.End = range.Begin + sizeof(vertexConstants);
	vertexConstantBuffer->Map(0, &range, (void**)&data);
	memcpy(data + currentConstantBuffer * sizeof(vertexConstants), vertexConstants, sizeof(vertexConstants));
	vertexConstantBuffer->Unmap(0, &range);

	range.Begin = currentConstantBuffer * sizeof(fragmentConstants);
	range.End = range.Begin + sizeof(fragmentConstants);
	fragmentConstantBuffer->Map(0, &range, (void**)&data);
	memcpy(data + currentConstantBuffer * sizeof(fragmentConstants), fragmentConstants, sizeof(fragmentConstants));
	fragmentConstantBuffer->Unmap(0, &range);

	_commandList->SetGraphicsRootConstantBufferView(1, vertexConstantBuffer->GetGPUVirtualAddress() + currentConstantBuffer * sizeof(vertexConstants));
	_commandList->SetGraphicsRootConstantBufferView(2, fragmentConstantBuffer->GetGPUVirtualAddress() + currentConstantBuffer * sizeof(fragmentConstants));
	
	++currentConstantBuffer;
	if (currentConstantBuffer >= constantBufferMultiply) {
		currentConstantBuffer = 0;
	}*/

	_commandList->DrawIndexedInstanced(count, 1, start, 0, 0);
}

void CommandList::executeAndWait() {
	graphicsFlushAndWait(_commandList, _commandAllocator, currentRenderTarget);
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

void CommandList::setVertexBuffers(VertexBuffer** buffers, int* offsets, int count) {
	buffers[0]->view.BufferLocation = buffers[0]->uploadBuffer->GetGPUVirtualAddress() + offsets[0] * buffers[0]->stride();
	buffers[0]->view.SizeInBytes = (buffers[0]->count() - offsets[0]) * buffers[0]->stride();
	_commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&buffers[0]->view);
}

void CommandList::setIndexBuffer(IndexBuffer& buffer) {
	_indexCount = buffer.count();
	_commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&buffer.indexBufferView);
}

void CommandList::setRenderTargets(RenderTarget** targets, int count) {
	currentRenderTarget = targets[0];
	graphicsFlushAndWait(_commandList, _commandAllocator, targets[0]);
	_commandList->OMSetRenderTargets(1, &targets[0]->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true,
		targets[0]->depthStencilDescriptorHeap != nullptr ? &targets[0]->depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart() : nullptr);
	_commandList->RSSetViewports(1, (D3D12_VIEWPORT*)&targets[0]->viewport);
	_commandList->RSSetScissorRects(1, (D3D12_RECT*)&targets[0]->scissor);
}

void CommandList::upload(IndexBuffer* buffer) {
	buffer->_upload(_commandList);
}

void CommandList::upload(Texture* texture) {
	D3D12_RESOURCE_DESC Desc = texture->image->GetDesc();
	ID3D12Device* device;
	texture->image->GetDevice(IID_GRAPHICS_PPV_ARGS(&device));
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->GetCopyableFootprints(&Desc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);
	device->Release();

	CD3DX12_TEXTURE_COPY_LOCATION source(texture->uploadImage, footprint);
	CD3DX12_TEXTURE_COPY_LOCATION destination(texture->image, 0);
	_commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
