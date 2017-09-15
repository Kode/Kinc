#include "pch.h"

#include "Metal.h"
#include "VertexBuffer5Impl.h"

#include <Kore/Math/Core.h>
#include <Kore/System.h>
#import <Metal/Metal.h>
#include <cstdio>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

int uniformsIndex = 0;
id<MTLBuffer> vertexUniforms;
id<MTLBuffer> fragmentUniforms;

namespace {
	// bool fullscreen;
	// TextureFilter minFilters[32];
	// MipmapFilter mipFilters[32];
	// int originalFramebuffer;
	const int uniformsSize = 4096;
	const int more = 5;
	
	void* vertexData(int offset) {
		u8* bytes = (u8*)[vertexUniforms contents];
		return &bytes[uniformsIndex * uniformsSize + offset];
	}
	
	void* fragmentData(int offset) {
		u8* bytes = (u8*)[fragmentUniforms contents];
		return &bytes[uniformsIndex * uniformsSize + offset];
	}
}

void Graphics5::destroy(int windowId) {

	System::destroyWindow(windowId);
}

void Graphics5::init(int, int, int, bool) {
	// System::createWindow();
	id<MTLDevice> device = getMetalDevice();
	vertexUniforms = [device newBufferWithLength:4096 * more options:MTLResourceOptionCPUCacheModeDefault];
	fragmentUniforms = [device newBufferWithLength:4096 * more options:MTLResourceOptionCPUCacheModeDefault];
}

void Graphics5::flush() {}

unsigned Graphics5::refreshRate() {
	return 60;
}

bool Graphics5::vsynced() {
	return true;
}

// void* Graphics::getControl() {
//	return nullptr;
//}

void Graphics5::setBool(ConstantLocation location, bool value) {
	if (location.vertexOffset >= 0) {
		int* ints = (int*)vertexData(location.vertexOffset);
		ints[0] = value ? 1 : 0;
	}
	if (location.fragmentOffset >= 0) {
		int* ints = (int*)fragmentData(location.vertexOffset);
		ints[0] = value ? 1 : 0;
	}
}

void Graphics5::setInt(ConstantLocation location, int value) {
	if (location.vertexOffset >= 0) {
		int* ints = (int*)vertexData(location.vertexOffset);
		ints[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		int* ints = (int*)fragmentData(location.vertexOffset);
		ints[0] = value;
	}
}

void Graphics5::setFloat(ConstantLocation location, float value) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		floats[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		floats[0] = value;
	}
}

void Graphics5::setFloat2(ConstantLocation location, float value1, float value2) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
	}
}

void Graphics5::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}
}

void Graphics5::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}
}

void Graphics5::setFloats(ConstantLocation location, float* values, int count) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}
}

void Graphics5::setMatrix(ConstantLocation location, const mat4& value) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		for (int i = 0; i < 16; ++i) {
			floats[i] = value.data[i];
		}
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		for (int i = 0; i < 16; ++i) {
			floats[i] = value.data[i];
		}
	}
}

void Graphics5::setMatrix(ConstantLocation location, const mat3& value) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		for (int i = 0; i < 9; ++i) {
			floats[i] = value.data[i];
		}
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		for (int i = 0; i < 9; ++i) {
			floats[i] = value.data[i];
		}
	}
}

void Graphics5::drawIndexedVerticesInstanced(int instanceCount) {
	
}

void Graphics5::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	
}

bool Graphics5::swapBuffers(int windowId) {
	System::swapBuffers(windowId);
	return true;
}

void beginGL();

void Graphics5::begin(RenderTarget* renderTarget, int windowId) {
	beginGL();
}

void Graphics5::end(int windowId) {
	++uniformsIndex;
	if (uniformsIndex >= more) {
		uniformsIndex = 0;
	}
}

void Graphics5::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {}

void Graphics5::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

void Graphics5::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {}

void Graphics5::setRenderTargetFace(RenderTarget* texture, int face) {}

bool Graphics5::renderTargetsInvertedY() {
	return true;
}

bool Graphics5::nonPow2TexturesSupported() {
	return true;
}

void Graphics5::setTexture(Graphics5::TextureUnit unit, Graphics5::Texture* texture) {
	texture->_set(unit);
}

void Graphics5::setImageTexture(TextureUnit unit, Texture* texture) {
	
}

bool Graphics5::initOcclusionQuery(uint* occlusionQuery) {
	return false;
}

void Graphics5::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics5::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics5::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics5::getQueryResults(uint occlusionQuery, uint* pixelCount) {}

void Graphics5::makeCurrent(int window) {
	
}

void Graphics5::clearCurrent() {
	
}

void Graphics5::changeResolution(int width, int height) {
	
}
