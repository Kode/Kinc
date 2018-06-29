#include "pch.h"

#include "IndexBuffer5Impl.h"
#include "PipelineState5Impl.h"
#include "VertexBuffer5Impl.h"
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/Math/Core.h>

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

namespace Kore {
	extern PipelineState5Impl* currentProgram;
}

void Graphics5::destroy(int window) {}

void Graphics5::init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	Graphics4::init(window, depthBufferBits, stencilBufferBits, vsync);
}

void Graphics5::changeResolution(int width, int height) {}

#if defined(KORE_WINDOWS) || defined(KORE_XBOX_ONE)
void Graphics5::setup() {}
#endif

void Graphics5::makeCurrent(int window) {}

void Graphics5::clearCurrent() {}

void Graphics5::drawIndexedVerticesInstanced(int instanceCount) {}

void Graphics5::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {}

void Graphics5::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {}

void Graphics5::begin(RenderTarget* renderTarget, int window) {}

void Graphics5::end(int window) {}

bool Graphics5::vsynced() {
	return Graphics4::vsynced();
}

unsigned Graphics5::refreshRate() {
	return Graphics4::refreshRate();
}

bool Graphics5::swapBuffers(int window) {
	return Graphics4::swapBuffers(window);
}

void Graphics5::flush() {}

void Graphics5::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {}

void Graphics5::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

bool Graphics5::renderTargetsInvertedY() {
	return false;
}

bool Graphics5::nonPow2TexturesSupported() {
	return true;
}

void Graphics5::setRenderTargetFace(RenderTarget* texture, int face) {}
/*
void Graphics5::setVertexBuffers(VertexBuffer** buffers, int count) {
    buffers[0]->_set(0);
}

void Graphics5::setIndexBuffer(IndexBuffer& buffer) {
    buffer._set();
}
*/
void Graphics5::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

bool Graphics5::initOcclusionQuery(uint* occlusionQuery) {
	return false;
}

void Graphics5::setImageTexture(TextureUnit unit, Texture* texture) {}

void Graphics5::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics5::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics5::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics5::getQueryResults(uint occlusionQuery, uint* pixelCount) {}

/*void Graphics5::setPipeline(PipelineState* pipeline) {
    pipeline->set(pipeline);
}*/
