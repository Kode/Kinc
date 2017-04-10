#include "pch.h"
#include <Kore/Compute/Compute.h>
#include "ogl.h"

using namespace Kore;

ShaderStorageBuffer* ShaderStorageBufferImpl::current = nullptr;

ShaderStorageBufferImpl::ShaderStorageBufferImpl(int count, VertexData type) : myCount(count) {

}

ShaderStorageBuffer::ShaderStorageBuffer(int indexCount, VertexData type) : ShaderStorageBufferImpl(indexCount, type) {
	myStride = 0;
	switch (type) {
	case ColorVertexData:
		myStride += 1 * 4;
		break;
	case Float1VertexData:
		myStride += 1 * 4;
		break;
	case Float2VertexData:
		myStride += 2 * 4;
		break;
	case Float3VertexData:
		myStride += 3 * 4;
		break;
	case Float4VertexData:
		myStride += 4 * 4;
		break;
	case Float4x4VertexData:
		myStride += 4 * 4 * 4;
		break;
	}

	glGenBuffers(1, &bufferId);
	glCheckErrors();
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
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
	glCheckErrors();
	glBufferData(GL_SHADER_STORAGE_BUFFER, myCount * myStride, data, GL_STATIC_DRAW);
	glCheckErrors();
}

void ShaderStorageBuffer::_set() {
	current = this;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
	glCheckErrors();
}

void ShaderStorageBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int ShaderStorageBuffer::count() {
	return myCount;
}
