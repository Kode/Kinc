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

namespace {
	unsigned hz;
	bool vsync;
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

void Graphics5::begin(RenderTarget* renderTarget, int window) {

}

void Graphics5::end(int window) {
	
}

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

namespace {
	void setInt(u8* constants, u32 offset, u32 size, int value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(u8* constants, u32 offset, u32 size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, u32 offset, u32 size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, u32 offset, u32 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, u32 offset, u32 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(u8* constants, u32 offset, u32 size, float* values, int count) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}

	void setBool(u8* constants, u32 offset, u32 size, bool value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat4& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat3& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}
}

void Graphics5::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

bool Graphics5::renderTargetsInvertedY() {
	return false;
}

bool Graphics5::nonPow2TexturesSupported() {
	return true;
}

void Graphics5::setRenderTargetFace(RenderTarget* texture, int face) {
	
}
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

void Graphics5::setImageTexture(TextureUnit unit, Texture* texture) {
	
}

void Graphics5::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics5::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics5::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics5::getQueryResults(uint occlusionQuery, uint* pixelCount) {}

/*void Graphics5::setPipeline(PipelineState* pipeline) {
	pipeline->set(pipeline);
}*/
