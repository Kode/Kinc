#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include "ShaderImpl.h"
#include "ogl.h"

using namespace Kore;

VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {

}

VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(vertexCount, instanceDataStepRate) {
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
		}
	}
	this->structure = structure;
	
	glGenBuffers(1, &bufferId);
	glCheckErrors();
	data = new float[vertexCount * myStride / 4];
}

VertexBuffer::~VertexBuffer() {
	unset();
	delete[] data;
}

float* VertexBuffer::lock() {
	return data;
}
/*
// TODO: FIXME!
float* VertexBuffer::lock(int start, int count) {
	myCount = count;
	return nullptr;//&buffer[start * 9];
}
*/

void VertexBuffer::unlock() {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glCheckErrors();
	glBufferData(GL_ARRAY_BUFFER, myStride * myCount, data, GL_STATIC_DRAW);
	glCheckErrors();
}

int VertexBuffer::_set(int offset) {
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();
	return offsetoffset;
}

void VertexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}

int VertexBufferImpl::setVertexAttributes(int offset) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glCheckErrors();

	int internaloffset = 0;
	int actualIndex = 0;
	for (int index = 0; index < structure.size; ++index) {
		VertexElement element = structure.elements[index];
		int size;
		switch (element.data) {
		case ColorVertexData:
			size = 1;
			break;
		case Float1VertexData:
			size = 1;
			break;
		case Float2VertexData:
			size = 2;
			break;
		case Float3VertexData:
			size = 3;
			break;
		case Float4VertexData:
			size = 4;
			break;
		case Float4x4VertexData:
			size = 16;
			break;
		}
		if (size > 4) {
			int subsize = size;
			int addonOffset = 0;
			while (subsize > 0) {
				glEnableVertexAttribArray(offset + actualIndex);
				glCheckErrors();
				glVertexAttribPointer(offset + actualIndex, 4, GL_FLOAT, false, myStride, (void*)(internaloffset + addonOffset));
				glCheckErrors();
#ifndef OPENGLES
				glVertexAttribDivisor(offset + actualIndex, instanceDataStepRate);
				glCheckErrors();
#endif
				subsize -= 4;
				addonOffset += 4 * 4;
				++actualIndex;
			}
		}
		else {
			glEnableVertexAttribArray(offset + actualIndex);
			glCheckErrors();
			glVertexAttribPointer(offset + actualIndex, size, GL_FLOAT, false, myStride, (void*)internaloffset);
			glCheckErrors();
#ifndef OPENGLES
			glVertexAttribDivisor(offset + actualIndex, instanceDataStepRate);
			glCheckErrors();
#endif
			++actualIndex;
		}
		switch (element.data) {
		case ColorVertexData:
			internaloffset += 4 * 1;
			break;
		case Float1VertexData:
			internaloffset += 4 * 1;
			break;
		case Float2VertexData:
			internaloffset += 4 * 2;
			break;
		case Float3VertexData:
			internaloffset += 4 * 3;
			break;
		case Float4VertexData:
			internaloffset += 4 * 4;
			break;
		case Float4x4VertexData:
			internaloffset += 4 * 4 * 4;
			break;
		}
	}
	for (int index = actualIndex; index < 16; ++index) {
		glDisableVertexAttribArray(offset + index);
		glCheckErrors();
	}
	return actualIndex;
}
