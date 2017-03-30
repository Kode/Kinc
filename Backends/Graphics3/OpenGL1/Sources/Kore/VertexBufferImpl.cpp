#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics3.h>
#include "ShaderImpl.h"
#include "ogl.h"
#include <assert.h>

using namespace Kore;

VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {
#ifndef NDEBUG
	initialized = false;
#endif
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
#ifndef NDEBUG
	initialized = true;
#endif
}

int VertexBuffer::_set(int offset) {
	assert(initialized); // Vertex Buffer is used before lock/unlock was called
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

    // Enable vertex attributes
    unsigned int usedAttribsMask = 0;

	int internaloffset = 0;
	int actualIndex = 0;

	for (int index = 0; index < structure.size; ++index) {
		VertexElement element = structure.elements[index];
		int    size = 0;
		GLenum type = GL_FLOAT;

		switch (element.data) {
		case ColorVertexData:
			size = 4;
			type = GL_UNSIGNED_BYTE;
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

        switch (element.attribute) {
            case VertexCoord:
                assert(size >= 2 && size <= 4);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case VertexNormal:
                assert(size == 3);
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case VertexColor0:
                assert(size >= 3 && size <= 4);
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case VertexColor1:
                assert(size == 3);
                glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
                glSecondaryColorPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case VertexTexCoord0:
            case VertexTexCoord1:
            case VertexTexCoord2:
            case VertexTexCoord3:
            case VertexTexCoord4:
            case VertexTexCoord5:
            case VertexTexCoord6:
            case VertexTexCoord7:
                assert(size >= 1 && size <= 4);
                glClientActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(element.attribute - VertexTexCoord0));
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            default:
                break;
        }

        usedAttribsMask |= (1u << element.attribute);
        ++actualIndex;

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

    // Disable unused vertex attributes
	for (int attrib = VertexCoord; attrib <= VertexTexCoord7; ++attrib) {
        if ((usedAttribsMask & (1u << attrib)) == 0) {
            switch (attrib) {
                case VertexCoord:
                    glDisableClientState(GL_VERTEX_ARRAY);
                    break;

                case VertexNormal:
                    glDisableClientState(GL_NORMAL_ARRAY);
                    break;

                case VertexColor0:
                    glDisableClientState(GL_COLOR_ARRAY);
                    break;

                case VertexColor1:
                    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
                    break;

                case VertexTexCoord0:
                case VertexTexCoord1:
                case VertexTexCoord2:
                case VertexTexCoord3:
                case VertexTexCoord4:
                case VertexTexCoord5:
                case VertexTexCoord6:
                case VertexTexCoord7:
                    glClientActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(attrib - VertexTexCoord0));
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    break;

                default:
                    break;
            }
        }
		glCheckErrors();
	}

	return actualIndex;
}
