#include "pch.h"

#include "G4.h"

#include "IndexBufferImpl.h"
#include "PipelineStateImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/ConstantBuffer.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>

using namespace Kore;

Graphics5::CommandList* commandList;

u64 frameNumber = 0;
bool waitAfterNextDraw = false;

namespace {
	const int bufferCount = 3;
	int currentBuffer = -1;
	Graphics5::RenderTarget* framebuffers[bufferCount];

	Graphics5::ConstantBuffer* vertexConstantBuffer;
	Graphics5::ConstantBuffer* fragmentConstantBuffer;
	const int constantBufferSize = 4096;
	const int constantBufferMultiply = 10;
	int constantBufferIndex = 0;
}

void Graphics4::destroy(int window) {
	Graphics5::destroy(window);
}

void Graphics4::init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	Graphics5::init(window, depthBufferBits, stencilBufferBits, vsync);
	commandList = new Graphics5::CommandList;
	for (int i = 0; i < bufferCount; ++i) {
		framebuffers[i] = new Graphics5::RenderTarget(System::windowWidth(window), System::windowHeight(window), depthBufferBits, false, Graphics5::Target32Bit, -1, -i - 1 /* hack in an index for backbuffer render targets */);
	}
	vertexConstantBuffer = new Graphics5::ConstantBuffer(constantBufferSize * constantBufferMultiply);
	fragmentConstantBuffer = new Graphics5::ConstantBuffer(constantBufferSize * constantBufferMultiply);
}

void Graphics4::changeResolution(int width, int height) {
	Graphics5::changeResolution(width, height);
}

void Graphics4::setup() {
	Graphics5::setup();
}

void Graphics4::makeCurrent(int window) {
	Graphics5::makeCurrent(window);
}

void Graphics4::clearCurrent() {
	Graphics5::clearCurrent();
}

namespace {
	void startDraw() {
		commandList->setPipelineLayout();
		vertexConstantBuffer->unlock();
		fragmentConstantBuffer->unlock();
		commandList->setVertexConstantBuffer(vertexConstantBuffer, constantBufferIndex * constantBufferSize);
		commandList->setFragmentConstantBuffer(fragmentConstantBuffer, constantBufferIndex * constantBufferSize);
	}

	void endDraw() {
		++constantBufferIndex;
		if (constantBufferIndex >= constantBufferMultiply || waitAfterNextDraw) {
			commandList->executeAndWait();
			constantBufferIndex = 0;
			waitAfterNextDraw = false;
		}
		vertexConstantBuffer->lock(constantBufferIndex * constantBufferSize, constantBufferSize);
		fragmentConstantBuffer->lock(constantBufferIndex * constantBufferSize, constantBufferSize);
	}
}

void Graphics4::drawIndexedVertices() {
	startDraw();
	commandList->drawIndexedVertices();
	endDraw();
}

void Graphics4::drawIndexedVertices(int start, int count) {
	startDraw();
	commandList->drawIndexedVertices(start, count);
	endDraw();
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {

	Graphics5::drawIndexedVerticesInstanced(instanceCount);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	Graphics5::drawIndexedVerticesInstanced(instanceCount, start, count);
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Graphics5::setTextureAddressing(unit._unit, (Graphics5::TexDir)dir, (Graphics5::TextureAddressing)addressing);
}

void Graphics4::setTexture3DAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Graphics4::setTextureAddressing(unit, dir, addressing);
}

void Graphics4::clear(uint flags, uint color, float depth, int stencil) {
	commandList->clear(framebuffers[currentBuffer], flags, color, depth, stencil);
}

void Graphics4::begin(int window) {
	currentBuffer = (currentBuffer + 1) % bufferCount;

	Graphics5::begin(framebuffers[currentBuffer], window);
	//commandList = new Graphics5::CommandList;
	commandList->begin();
	commandList->framebufferToRenderTargetBarrier(framebuffers[currentBuffer]);
	commandList->setRenderTargets(&framebuffers[currentBuffer], 1);

	constantBufferIndex = 0;
	vertexConstantBuffer->lock(0, constantBufferSize);
	fragmentConstantBuffer->lock(0, constantBufferSize);

	++frameNumber;
}

void Graphics4::viewport(int x, int y, int width, int height) {
	commandList->viewport(x, y, width, height);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	commandList->scissor(x, y, width, height);
}

void Graphics4::disableScissor() {
	commandList->disableScissor();
}

void Graphics4::end(int window) {
	vertexConstantBuffer->unlock();
	fragmentConstantBuffer->unlock();

	commandList->renderTargetToFramebufferBarrier(framebuffers[currentBuffer]);
	commandList->end();
	//delete commandList;
	//commandList = nullptr;
	Graphics5::end(window);
}

bool Graphics4::vsynced() {
	return Graphics5::vsynced();
}

unsigned Graphics4::refreshRate() {
	return Graphics5::refreshRate();
}

bool Graphics4::swapBuffers(int window) {
	return Graphics5::swapBuffers(window);
}

void Graphics4::flush() {
	return Graphics5::flush();
}

void Graphics4::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	Graphics5::setTextureOperation((Graphics5::TextureOperation)operation, (Graphics5::TextureArgument)arg1, (Graphics5::TextureArgument)arg2);
}

void Graphics4::setInt(ConstantLocation location, int value) {
	vertexConstantBuffer->setInt(0, value);
	fragmentConstantBuffer->setInt(0, value);
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	vertexConstantBuffer->setFloat(0, value);
	fragmentConstantBuffer->setFloat(0, value);
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	vertexConstantBuffer->setFloat2(0, value1, value2);
	fragmentConstantBuffer->setFloat2(0, value1, value2);
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	vertexConstantBuffer->setFloat3(0, value1, value2, value3);
	fragmentConstantBuffer->setFloat3(0, value1, value2, value3);
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	vertexConstantBuffer->setFloat4(0, value1, value2, value3, value4);
	fragmentConstantBuffer->setFloat4(0, value1, value2, value3, value4);
}

void Graphics4::setFloats(ConstantLocation location, float* values, int count) {
	vertexConstantBuffer->setFloats(0, values, count);
	fragmentConstantBuffer->setFloats(0, values, count);
}

void Graphics4::setBool(ConstantLocation location, bool value) {
	vertexConstantBuffer->setBool(0, value);
	fragmentConstantBuffer->setBool(0, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	vertexConstantBuffer->setMatrix(0, value);
	fragmentConstantBuffer->setMatrix(0, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	vertexConstantBuffer->setMatrix(0, value);
	fragmentConstantBuffer->setMatrix(0, value);
}

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics5::setTextureMagnificationFilter(texunit._unit, (Graphics5::TextureFilter)filter);
}

void Graphics4::setTexture3DMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMagnificationFilter(texunit, filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics5::setTextureMinificationFilter(texunit._unit, (Graphics5::TextureFilter)filter);
}

void Graphics4::setTexture3DMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMinificationFilter(texunit, filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Graphics5::setTextureMipmapFilter(texunit._unit, (Graphics5::MipmapFilter)filter);
}

void Graphics4::setTexture3DMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Graphics4::setTextureMipmapFilter(texunit, filter);
}

bool Graphics4::renderTargetsInvertedY() {
	return Graphics5::renderTargetsInvertedY();
}

bool Graphics4::nonPow2TexturesSupported() {
	return Graphics5::nonPow2TexturesSupported();
}

void Graphics4::restoreRenderTarget() {
	commandList->setRenderTargets(&framebuffers[currentBuffer], 1);
}

void Graphics4::setRenderTargets(RenderTarget** targets, int count) {
	Graphics5::RenderTarget* renderTargets[16];
	for (int i = 0; i < count; ++i) {
		renderTargets[i] = &targets[i]->_renderTarget;
		commandList->textureToRenderTargetBarrier(renderTargets[i]);
	}
	commandList->setRenderTargets(renderTargets, count);
}

void Graphics4::setRenderTargetFace(RenderTarget* texture, int face) {
	Graphics5::setRenderTargetFace(&texture->_renderTarget, face);
}

void Graphics4::setVertexBuffers(VertexBuffer** buffers, int count) {
	Graphics5::VertexBuffer* g5buffers[16];
	int offsets[16];
	for (int i = 0; i < count; ++i) {
		g5buffers[i] = &buffers[i]->_buffer;
		int index = buffers[i]->_currentIndex;
		offsets[i] = index * buffers[i]->count();
	}
	commandList->setVertexBuffers(g5buffers, offsets, count);
}

void Graphics4::setIndexBuffer(IndexBuffer& buffer) {
	commandList->setIndexBuffer(buffer._buffer);
}

void Graphics4::setTexture(TextureUnit unit, Texture* texture) {
	if (!texture->_uploaded) {
		commandList->upload(texture->_texture);
		texture->_uploaded = true;
	}
	Graphics5::setTexture(unit._unit, texture->_texture);
}

void Graphics4::setImageTexture(Kore::Graphics4::TextureUnit unit, Kore::Graphics4::Texture *texture) {
	
}

bool Graphics4::initOcclusionQuery(uint* occlusionQuery) {
	return Graphics5::initOcclusionQuery(occlusionQuery);
}

void Graphics4::deleteOcclusionQuery(uint occlusionQuery) {
	Graphics5::deleteOcclusionQuery(occlusionQuery);
}

void Graphics4::renderOcclusionQuery(uint occlusionQuery, int triangles) {
	Graphics5::renderOcclusionQuery(occlusionQuery, triangles);
}

bool Graphics4::isQueryResultsAvailable(uint occlusionQuery) {
	return Graphics5::isQueryResultsAvailable(occlusionQuery);
}

void Graphics4::getQueryResults(uint occlusionQuery, uint* pixelCount) {
	Graphics5::getQueryResults(occlusionQuery, pixelCount);
}

void Graphics4::setPipeline(PipelineState* pipeline) {
	commandList->setPipeline(pipeline->_pipeline);
}

void Graphics4::setTextureArray(TextureUnit unit, TextureArray* array) {

}
