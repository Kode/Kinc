#include "pch.h"

#include <Kinc/Graphics5/CommandList.h>
#include <Kinc/Graphics5/ConstantBuffer.h>
#include <Kinc/Graphics5/IndexBuffer.h>
#include <Kinc/Graphics5/Pipeline.h>
#include <Kinc/Graphics5/VertexBuffer.h>

#include "Direct3D12.h"

#include <type_traits>

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

	void graphicsFlush(kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator, kinc_g5_render_target_t *renderTarget) {
		list->impl._commandList->Close();
		list->impl.closed = true;

		ID3D12CommandList *commandLists[] = {list->impl._commandList};
		commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);

		commandQueue->Signal(renderFence, ++renderFenceValue);
	}

	void graphicsWait(kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator, kinc_g5_render_target_t *renderTarget) {
		waitForFence(renderFence, renderFenceValue, renderFenceEvent);
		commandAllocator->Reset();
		list->impl._commandList->Reset(commandAllocator, nullptr);
		list->impl._commandList->OMSetRenderTargets(1, &renderTarget->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
		list->impl._commandList->RSSetViewports(1, (D3D12_VIEWPORT *)&renderTarget->impl.viewport);
		list->impl._commandList->RSSetScissorRects(1, (D3D12_RECT *)&renderTarget->impl.scissor);
	}

	void graphicsFlushAndWait(kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator, kinc_g5_render_target_t *renderTarget) {
		graphicsFlush(list, commandAllocator, renderTarget);
		graphicsWait(list, commandAllocator, renderTarget);
	}

	int formatSize(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return 8;
		case DXGI_FORMAT_R16_FLOAT:
			return 2;
		case DXGI_FORMAT_R8_UNORM:
			return 1;
		default:
			return 4;
		}
	}

	kinc_g5_render_target_t *currentRenderTarget = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE targetDescriptors[16];
}

void kinc_g5_command_list_init(kinc_g5_command_list *list) {
	::init();
	list->impl.closed = false;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&list->impl._commandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->impl._commandAllocator, nullptr, IID_GRAPHICS_PPV_ARGS(&list->impl._commandList));
	//_commandList->Close();
	// createConstantBuffer();
	list->impl._indexCount = 0;
}

void kinc_g5_command_list_destroy(kinc_g5_command_list *list) {}

void kinc_g5_command_list_begin(kinc_g5_command_list *list) {
	if (list->impl.closed) {
		list->impl.closed = false;
		waitForFence(renderFence, list->impl.currentFenceValue, renderFenceEvent);
		list->impl._commandAllocator->Reset();
		list->impl._commandList->Reset(list->impl._commandAllocator, nullptr);
	}
}

void kinc_g5_command_list_end(kinc_g5_command_list *list) {
	graphicsFlush(list, list->impl._commandAllocator, currentRenderTarget);

	list->impl.currentFenceValue = ++renderFenceValue;
	commandQueue->Signal(renderFence, list->impl.currentFenceValue);
}

void kinc_g5_command_list_clear(kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget, unsigned flags, unsigned color, float depth, int stencil) {
	if (flags & KINC_G5_CLEAR_COLOR) {
		float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f,
							  ((color & 0x0000ff00) >> 8) / 255.0f,
							   (color & 0x000000ff) / 255.0f,
							  ((color & 0xff000000) >> 24) / 255.0f};
		list->impl._commandList->ClearRenderTargetView(renderTarget->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}
	if ((flags & KINC_G5_CLEAR_DEPTH) || (flags & KINC_G5_CLEAR_STENCIL)) {
		D3D12_CLEAR_FLAGS d3dflags = (flags & KINC_G5_CLEAR_DEPTH) && (flags & KINC_G5_CLEAR_STENCIL)
		                                 ? D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL
		                                 : (flags & KINC_G5_CLEAR_DEPTH) ? D3D12_CLEAR_FLAG_DEPTH : D3D12_CLEAR_FLAG_STENCIL;
		list->impl._commandList->ClearDepthStencilView(renderTarget->impl.depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), d3dflags, depth, stencil,
		                                               0, nullptr);
	}
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->ResourceBarrier(1, &barrier);
}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->ResourceBarrier(1, &barrier);
}

void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	if (renderTarget->impl.resourceState != RenderTargetResourceStateRenderTarget) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->ResourceBarrier(1, &barrier);
		renderTarget->impl.resourceState = RenderTargetResourceStateRenderTarget;
	}
}

void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	if (renderTarget->impl.resourceState != RenderTargetResourceStateTexture) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->ResourceBarrier(1, &barrier);
		renderTarget->impl.resourceState = RenderTargetResourceStateTexture;
	}
}

void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list *list) {
	kinc_g5_internal_setConstants(list->impl._commandList, list->impl._currentPipeline);
}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
	list->impl._commandList->SetGraphicsRootConstantBufferView(2, buffer->impl._buffer->GetGPUVirtualAddress() + offset);
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
	list->impl._commandList->SetGraphicsRootConstantBufferView(3, buffer->impl._buffer->GetGPUVirtualAddress() + offset);
}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list *list, int start, int count) {
	list->impl._commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	list->impl._commandList->DrawIndexedInstanced(count, 1, start, 0, 0);
}

void kinc_g5_command_list_execute_and_wait(kinc_g5_command_list *list) {
	graphicsFlushAndWait(list, list->impl._commandAllocator, currentRenderTarget);
}

void kinc_g5_command_list_viewport(kinc_g5_command_list *list, int x, int y, int width, int height) {
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = static_cast<float>(x);
	viewport.TopLeftY = static_cast<float>(y);
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	list->impl._commandList->RSSetViewports(1, &viewport);
}

void kinc_g5_command_list_scissor(kinc_g5_command_list *list, int x, int y, int width, int height) {
	/*D3D12_RECT scissor;
	scissor.left = x;
	scissor.top = y;
	scissor.right = x + width;
	scissor.bottom = y + height;
	list->impl._commandList->RSSetScissorRects(1, &scissor);*/
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list *list) {}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list *list, kinc_g5_pipeline_t *pipeline) {
	list->impl._currentPipeline = pipeline;
	list->impl._commandList->SetPipelineState(pipeline->impl.pso);
}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list *list, kinc_g5_vertex_buffer_t **buffers, int *offsets, int count) {
	buffers[0]->impl.view.BufferLocation = buffers[0]->impl.uploadBuffer->GetGPUVirtualAddress() + offsets[0] * kinc_g5_vertex_buffer_stride(buffers[0]);
	buffers[0]->impl.view.SizeInBytes = (kinc_g5_vertex_buffer_count(buffers[0]) - offsets[0]) * kinc_g5_vertex_buffer_stride(buffers[0]);
	list->impl._commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW *)&buffers[0]->impl.view);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list *list, kinc_g5_index_buffer_t *buffer) {
	list->impl._indexCount = kinc_g5_index_buffer_count(buffer);
	list->impl._commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW *)&buffer->impl.indexBufferView);
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list *list, kinc_g5_render_target_t **targets, int count) {
	currentRenderTarget = targets[0];
	graphicsFlushAndWait(list, list->impl._commandAllocator, targets[0]);
	for (int i = 0; i < count; ++i) {
		targetDescriptors[i] = targets[i]->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	}
	list->impl._commandList->OMSetRenderTargets(
	    count, &targetDescriptors[0], false,
	    targets[0]->impl.depthStencilDescriptorHeap != nullptr ? &targets[0]->impl.depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart() : nullptr);
	list->impl._commandList->RSSetViewports(1, (D3D12_VIEWPORT *)&targets[0]->impl.viewport);
	list->impl._commandList->RSSetScissorRects(1, (D3D12_RECT *)&targets[0]->impl.scissor);
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, kinc_g5_index_buffer_t *buffer) {
	kinc_g5_internal_index_buffer_upload(buffer, list->impl._commandList);
}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, kinc_g5_texture_t *texture) {
	D3D12_RESOURCE_DESC Desc = texture->impl.image->GetDesc();
	ID3D12Device* device;
	texture->impl.image->GetDevice(IID_GRAPHICS_PPV_ARGS(&device));
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->GetCopyableFootprints(&Desc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);
	device->Release();

	CD3DX12_TEXTURE_COPY_LOCATION source(texture->impl.uploadImage, footprint);
	CD3DX12_TEXTURE_COPY_LOCATION destination(texture->impl.image, 0);
	list->impl._commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	list->impl._commandList->ResourceBarrier(
	    1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->impl.image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
static int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
#else
int d3d12_textureAlignment();
#endif

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	DXGI_FORMAT dxgiFormat = render_target->impl.renderTarget->GetDesc().Format;
	int formatByteSize = formatSize(dxgiFormat);

	// Create readback buffer
	if (render_target->impl.renderTargetReadback == nullptr) {
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(render_target->texWidth * render_target->texHeight * formatByteSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTargetReadback));
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_STATES sourceState = render_target->impl.resourceState == RenderTargetResourceStateRenderTarget ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = sourceState;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->ResourceBarrier(1, &barrier);
	}

	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = render_target->impl.renderTarget;
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = render_target->impl.renderTargetReadback;
	dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dest.PlacedFootprint.Offset = 0;
	dest.PlacedFootprint.Footprint.Format = dxgiFormat;
	dest.PlacedFootprint.Footprint.Width = render_target->texWidth;
	dest.PlacedFootprint.Footprint.Height = render_target->texHeight;
	dest.PlacedFootprint.Footprint.Depth = 1;
	int rowPitch = render_target->texWidth * formatByteSize;
	int align = rowPitch % d3d12_textureAlignment();
	if (align != 0) rowPitch = rowPitch + (d3d12_textureAlignment() - align);
	dest.PlacedFootprint.Footprint.RowPitch = rowPitch;

	list->impl._commandList->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = sourceState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->ResourceBarrier(1, &barrier);
	}

	graphicsFlushAndWait(list, list->impl._commandAllocator, currentRenderTarget);

	// Read buffer
	void* p;
	render_target->impl.renderTargetReadback->Map(0, nullptr, &p);
	memcpy(data, p, render_target->texWidth * render_target->texHeight * formatByteSize);
	render_target->impl.renderTargetReadback->Unmap(0, nullptr);
}
