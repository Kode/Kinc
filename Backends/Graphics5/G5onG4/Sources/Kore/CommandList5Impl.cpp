#include "pch.h"

#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/PipelineState.h>

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;
using namespace Kore::Graphics5;

namespace {
	enum Commands {
		Clear,
		Draw,
		SetViewport,
		SetScissor,
		SetPipeline,
		SetVertexBuffer,
		SetIndexBuffer,
		SetRenderTarget
	};
}

CommandList::CommandList() {

}

CommandList::~CommandList() {

}

void CommandList::begin() {
	commandIndex = 0;
}

void CommandList::end() {
	int index = 0;
	while (index < commandIndex) {
		switch (commands[index]) {
		case Clear:
			Graphics4::clear((uint)commands[index + 1], (uint)commands[index + 2]);
			index += 3;
			break;
		case Draw:
			Graphics4::drawIndexedVertices((int)commands[index + 1], (int)commands[index + 2]);
			index += 3;
			break;
		case SetViewport:
			Graphics4::viewport((int)commands[index + 1], (int)commands[index + 2], (int)commands[index + 3], (int)commands[index + 4]);
			index += 5;
			break;
		case SetScissor:
			Graphics4::scissor((int)commands[index + 1], (int)commands[index + 2], (int)commands[index + 3], (int)commands[index + 4]);
			index += 5;
			break;
		case SetPipeline: {
			PipelineState* pipeline = (PipelineState*)commands[index + 1];
			Graphics4::setPipeline(pipeline->state);
			index += 2;
			break;
		}
		case SetVertexBuffer: {
			VertexBuffer* vb = (VertexBuffer*)commands[index + 1];
			Graphics4::setVertexBuffer(*vb->buffer);
			index += 2;
			break;
		}
		case SetIndexBuffer: {
			IndexBuffer* ib = (IndexBuffer*)commands[index + 1];
			Graphics4::setIndexBuffer(*ib->buffer);
			index += 2;
			break;
		}
		case SetRenderTarget:

			index += 2;
			break;
		default:
			return;
		}
	}
}

void CommandList::clear(RenderTarget* renderTarget, uint flags, uint color, float depth, int stencil) {
	commands[commandIndex++] = Clear;
	commands[commandIndex++] = flags;
	commands[commandIndex++] = color;
}

void CommandList::renderTargetToFramebufferBarrier(RenderTarget* renderTarget) {
	
}

void CommandList::framebufferToRenderTargetBarrier(RenderTarget* renderTarget) {
	
}

void CommandList::drawIndexedVertices() {
	commands[commandIndex++] = Draw;
	commands[commandIndex++] = 0;
	commands[commandIndex++] = _indexCount;
}

void CommandList::drawIndexedVertices(int start, int count) {
	commands[commandIndex++] = Draw;
	commands[commandIndex++] = start;
	commands[commandIndex++] = count;
}

void CommandList::viewport(int x, int y, int width, int height) {
	commands[commandIndex++] = SetViewport;
	commands[commandIndex++] = x;
	commands[commandIndex++] = y;
	commands[commandIndex++] = width;
	commands[commandIndex++] = height;
}

void CommandList::scissor(int x, int y, int width, int height) {
	commands[commandIndex++] = SetScissor;
	commands[commandIndex++] = x;
	commands[commandIndex++] = y;
	commands[commandIndex++] = width;
	commands[commandIndex++] = height;
}

void CommandList::disableScissor() {
	
}

void CommandList::setPipeline(PipelineState* pipeline) {
	commands[commandIndex++] = SetPipeline;
	commands[commandIndex++] = (s64)pipeline;
}

void CommandList::setVertexBuffers(VertexBuffer** buffers, int* offsets, int count) {
	commands[commandIndex++] = SetVertexBuffer;
	commands[commandIndex++] = (s64)buffers[0];
}

void CommandList::setIndexBuffer(IndexBuffer& buffer) {
	commands[commandIndex++] = SetIndexBuffer;
	commands[commandIndex++] = (s64)&buffer;
}

void CommandList::setRenderTargets(RenderTarget** targets, int count) {
	commands[commandIndex++] = SetRenderTarget;
	commands[commandIndex++] = (s64)targets[0];
}

void CommandList::upload(IndexBuffer* buffer) {
	
}

void CommandList::upload(Texture* texture) {
	
}
