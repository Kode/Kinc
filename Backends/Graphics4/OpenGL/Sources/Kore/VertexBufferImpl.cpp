#include "pch.h"

#include "ShaderImpl.h"
#include "VertexBufferImpl.h"
#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>
#include <assert.h>

using namespace Kore;

Graphics4::VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {
#ifndef NDEBUG
	initialized = false;
#endif
}

Graphics4::VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(vertexCount, instanceDataStepRate) {
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		VertexElement element = structure.elements[i];
		switch (element.data) {
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
		case NoVertexData:
			break;
		}
	}
	this->structure = structure;

	glGenBuffers(1, &bufferId);
	glCheckErrors();
	data = new float[vertexCount * myStride / 4];
}

Graphics4::VertexBuffer::~VertexBuffer() {
	unset();
	delete[] data;
}

float* Graphics4::VertexBuffer::lock() {
	return data;
}

float* Graphics4::VertexBuffer::lock(int start, int count) {
	u8* u8data = (u8*)data;
	return (float*)&u8data[start * stride()];
}

void Graphics4::VertexBuffer::unlock() {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glCheckErrors();
	glBufferData(GL_ARRAY_BUFFER, myStride * myCount, data, GL_STATIC_DRAW);
	glCheckErrors();
#ifndef NDEBUG
	initialized = true;
#endif
}

int Graphics4::VertexBuffer::_set(int offset) {
	assert(initialized); // Vertex Buffer is used before lock/unlock was called
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();
	return offsetoffset;
}

void VertexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int Graphics4::VertexBuffer::count() {
	return myCount;
}

int Graphics4::VertexBuffer::stride() {
	return myStride;
}

#ifndef KORE_OPENGL_ES
namespace {
	bool attribDivisorUsed = false;
}
#endif

int VertexBufferImpl::setVertexAttributes(int offset) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glCheckErrors();

	int internaloffset = 0;
	int actualIndex = 0;
	for (int index = 0; index < structure.size; ++index) {
		Graphics4::VertexElement element = structure.elements[index];
		int size = 0;
		GLenum type = GL_FLOAT;
		switch (element.data) {
		case Graphics4::ColorVertexData:
			size = 4;
			type = GL_UNSIGNED_BYTE;
			break;
		case Graphics4::Float1VertexData:
			size = 1;
			break;
		case Graphics4::Float2VertexData:
			size = 2;
			break;
		case Graphics4::Float3VertexData:
			size = 3;
			break;
		case Graphics4::Float4VertexData:
			size = 4;
			break;
		case Graphics4::Float4x4VertexData:
			size = 16;
			break;
		case Graphics4::NoVertexData:
			break;
		}
		if (size > 4) {
			int subsize = size;
			int addonOffset = 0;
			while (subsize > 0) {
				glEnableVertexAttribArray(offset + actualIndex);
				glCheckErrors();
				glVertexAttribPointer(offset + actualIndex, 4, type, false, myStride, reinterpret_cast<void*>(internaloffset + addonOffset));
				glCheckErrors();
#ifndef KORE_OPENGL_ES
				if (attribDivisorUsed || instanceDataStepRate != 0) {
					attribDivisorUsed = true;
					glVertexAttribDivisor(offset + actualIndex, instanceDataStepRate);
					glCheckErrors();
				}
#endif
				subsize -= 4;
				addonOffset += 4 * 4;
				++actualIndex;
			}
		}
		else {
			glEnableVertexAttribArray(offset + actualIndex);
			glCheckErrors();
			glVertexAttribPointer(offset + actualIndex, size, type, false, myStride, reinterpret_cast<void*>(internaloffset));
			glCheckErrors();
#ifndef KORE_OPENGL_ES
			if (attribDivisorUsed || instanceDataStepRate != 0) {
				attribDivisorUsed = true;
				glVertexAttribDivisor(offset + actualIndex, instanceDataStepRate);
				glCheckErrors();
			}
#endif
			++actualIndex;
		}
		switch (element.data) {
		case Graphics4::ColorVertexData:
			internaloffset += 4 * 1;
			break;
		case Graphics4::Float1VertexData:
			internaloffset += 4 * 1;
			break;
		case Graphics4::Float2VertexData:
			internaloffset += 4 * 2;
			break;
		case Graphics4::Float3VertexData:
			internaloffset += 4 * 3;
			break;
		case Graphics4::Float4VertexData:
			internaloffset += 4 * 4;
			break;
		case Graphics4::Float4x4VertexData:
			internaloffset += 4 * 4 * 4;
			break;
		case Graphics4::NoVertexData:
			break;
		}
	}
	for (int index = actualIndex; index < 16; ++index) {
		glDisableVertexAttribArray(offset + index); // TODO: Can cause problems, please anaylze
		glCheckErrors();
	}
	return actualIndex;
}
