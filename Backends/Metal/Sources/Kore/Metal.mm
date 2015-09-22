#include "pch.h"
#include "Metal.h"
#include "VertexBufferImpl.h"
#include <Kore/Application.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <cstdio>
#import <Metal/Metal.h>

using namespace Kore;

namespace {
	//bool fullscreen;
	//TextureFilter minFilters[32];
	//MipmapFilter mipFilters[32];
	//int originalFramebuffer;
	id <MTLBuffer> vertexUniforms;
	id <MTLBuffer> fragmentUniforms;
	const int uniformsSize = 4096;
	const int more = 5;
	int uniformsIndex = 0;
	
	void* vertexData(int offset) {
		u8* bytes = (u8*)[vertexUniforms contents];
		return &bytes[uniformsIndex * uniformsSize + offset];
	}
	
	void* fragmentData(int offset) {
		u8* bytes = (u8*)[fragmentUniforms contents];
		return &bytes[uniformsIndex * uniformsSize + offset];
	}
}

id getMetalDevice();
id getMetalEncoder();

void Graphics::destroy() {

	System::destroyWindow();
}

#undef CreateWindow

void Graphics::init() {
	System::createWindow();
	id <MTLDevice> device = getMetalDevice();
	vertexUniforms = [device newBufferWithLength:4096 * more options:MTLResourceOptionCPUCacheModeDefault];
	fragmentUniforms = [device newBufferWithLength:4096 * more options:MTLResourceOptionCPUCacheModeDefault];
}

void Graphics::flush() {
	
}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::vsynced() {
	return true;
}

void* Graphics::getControl() {
	return nullptr;
}

void Graphics::setBool(ConstantLocation location, bool value) {
	if (location.vertexOffset >= 0) {
		int* ints = (int*)vertexData(location.vertexOffset);
		ints[0] = value ? 1 : 0;
	}
	if (location.fragmentOffset >= 0) {
		int* ints = (int*)fragmentData(location.vertexOffset);
		ints[0] = value ? 1 : 0;
	}
}

void Graphics::setInt(ConstantLocation location, int value) {
	if (location.vertexOffset >= 0) {
		int* ints = (int*)vertexData(location.vertexOffset);
		ints[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		int* ints = (int*)fragmentData(location.vertexOffset);
		ints[0] = value;
	}
}

void Graphics::setFloat(ConstantLocation location, float value) {
	if (location.vertexOffset >= 0) {
		float* floats = (float*)vertexData(location.vertexOffset);
		floats[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		float* floats = (float*)fragmentData(location.vertexOffset);
		floats[0] = value;
	}
}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {
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

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
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

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
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

void Graphics::setFloats(ConstantLocation location, float* values, int count) {
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

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
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

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {
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

void Graphics::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {
	id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	
	//[encoder setDepthStencilState:_depthState];
	[encoder setVertexBuffer:vertexUniforms offset:uniformsIndex * uniformsSize atIndex:1];
	[encoder setFragmentBuffer:fragmentUniforms offset:uniformsIndex * uniformsSize atIndex:0];
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:IndexBufferImpl::current->mtlBuffer indexBufferOffset:start + IndexBufferImpl::current->offset()];
}

void Graphics::swapBuffers() {
	System::swapBuffers();
}

void beginGL();

void Graphics::begin() {
	beginGL();
	
}

void Graphics::viewport(int x, int y, int width, int height) {
	//TODO
	// id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	// MTLViewport viewport;
	// viewport.originX=x;
	// viewport.originY=y;
	// viewport.width=width;
	// viewport.height=height;
	// encoder.setViewport(viewport);
}

void Graphics::end() {
	++uniformsIndex;
	if (uniformsIndex >= more) {
		uniformsIndex = 0;
	}
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {

}

void Graphics::setRenderState(RenderState state, bool on) {

}

void Graphics::setRenderState(RenderState state, int v) {

}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {

}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {

}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {

}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {

}

void Graphics::setRenderTarget(RenderTarget* texture, int num) {

}

void Graphics::restoreRenderTarget() {

}

bool Graphics::renderTargetsInvertedY() {
	return true;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}
