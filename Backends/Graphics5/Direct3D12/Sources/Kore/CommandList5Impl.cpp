#include "pch.h"

#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/PipelineState.h>

#include "Direct3D12.h"

#include "d3dx12.h"

using namespace Kore;
using namespace Kore::Graphics5;

CommandList::CommandList() {
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_PPV_ARGS(&_commandList));
	//_commandList->Close();
	_indexCount = 0;
}

CommandList::~CommandList() {

}

void CommandList::drawIndexedVertices() {
	drawIndexedVertices(0, _indexCount);
}

void CommandList::drawIndexedVertices(int start, int count) {
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
	_commandList->SetPipelineState(pipeline->pso);
}

void CommandList::setVertexBuffers(VertexBuffer** buffers, int count) {
	_commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&buffers[0]->view);
}

void CommandList::setIndexBuffer(IndexBuffer& buffer) {
	_indexCount = buffer.count();
	_commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&buffer.indexBufferView);
}
