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

void CommandList::setVertexBuffers(VertexBuffer** buffers, int count) {
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
