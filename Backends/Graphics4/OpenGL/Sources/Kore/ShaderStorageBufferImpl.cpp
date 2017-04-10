#include "pch.h"
#include <Kore/Compute/Compute.h>
#include "ogl.h"

using namespace Kore;

#if defined(KORE_WINDOWS) || (defined(KORE_LINUX) && defined(GL_VERSION_4_3)) || (defined(KORE_ANDROID) && defined(GL_ES_VERSION_3_1))
#define HAS_COMPUTE
#endif

ShaderStorageBuffer* ShaderStorageBufferImpl::current = nullptr;

ShaderStorageBufferImpl::ShaderStorageBufferImpl(int count, Graphics4::VertexData type) : myCount(count) {

}

ShaderStorageBuffer::ShaderStorageBuffer(int indexCount, Graphics4::VertexData type) : ShaderStorageBufferImpl(indexCount, type) {
	myStride = 0;
	switch (type) {
	case Graphics4::ColorVertexData:
		myStride += 1 * 4;
		break;
	case Graphics4::Float1VertexData:
		myStride += 1 * 4;
		break;
	case Graphics4::Float2VertexData:
		myStride += 2 * 4;
		break;
	case Graphics4::Float3VertexData:
		myStride += 3 * 4;
		break;
	case Graphics4::Float4VertexData:
		myStride += 4 * 4;
		break;
	case Graphics4::Float4x4VertexData:
		myStride += 4 * 4 * 4;
		break;
	}
#ifdef HAS_COMPUTE
	glGenBuffers(1, &bufferId);
	glCheckErrors();
#endif
	data = new int[indexCount];
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	unset();
	delete[] data;
}

int* ShaderStorageBuffer::lock() {
	return data;
}

void ShaderStorageBuffer::unlock() {
#ifdef HAS_COMPUTE
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
	glCheckErrors();
	glBufferData(GL_SHADER_STORAGE_BUFFER, myCount * myStride, data, GL_STATIC_DRAW);
	glCheckErrors();
#endif
}

void ShaderStorageBuffer::_set() {
	current = this;
#ifdef HAS_COMPUTE
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
	glCheckErrors();
#endif
}

void ShaderStorageBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int ShaderStorageBuffer::count() {
	return myCount;
}
