#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics3/Graphics.h>

#include "ShaderImpl.h"
#include "ogl.h"

#include <assert.h>

using namespace Kore;

Graphics3::VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {
#ifndef NDEBUG
	initialized = false;
#endif
}

Graphics3::VertexBuffer::VertexBuffer(int vertexCount, const Graphics4::VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(vertexCount, instanceDataStepRate) {
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		Graphics4::VertexElement element = structure.elements[i];
		switch (element.data) {
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
	}
	this->structure = structure;

	glGenBuffers(1, &bufferId);
	glCheckErrors();
	data = new float[vertexCount * myStride / 4];
}

Graphics3::VertexBuffer::~VertexBuffer() {
	unset();
	delete[] data;
}

float* Graphics3::VertexBuffer::lock() {
	return data;
}
/*
// TODO: FIXME!
float* VertexBuffer::lock(int start, int count) {
	myCount = count;
	return nullptr;//&buffer[start * 9];
}
*/

void Graphics3::VertexBuffer::unlock() {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glCheckErrors();
	glBufferData(GL_ARRAY_BUFFER, myStride * myCount, data, GL_STATIC_DRAW);
	glCheckErrors();
#ifndef NDEBUG
	initialized = true;
#endif
}

int Graphics3::VertexBuffer::_set(int offset) {
	assert(initialized); // Vertex Buffer is used before lock/unlock was called
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();
	return offsetoffset;
}

void VertexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int Graphics3::VertexBuffer::count() {
	return myCount;
}

int Graphics3::VertexBuffer::stride() {
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
		Graphics4::VertexElement element = structure.elements[index];
		int    size = 0;
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
		}

        switch (element.attribute) {
            case Graphics4::VertexCoord:
                assert(size >= 2 && size <= 4);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case Graphics4::VertexNormal:
                assert(size == 3);
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case Graphics4::VertexColor0:
                assert(size >= 3 && size <= 4);
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case Graphics4::VertexColor1:
                assert(size == 3);
                glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
                glSecondaryColorPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            case Graphics4::VertexTexCoord0:
            case Graphics4::VertexTexCoord1:
            case Graphics4::VertexTexCoord2:
            case Graphics4::VertexTexCoord3:
            case Graphics4::VertexTexCoord4:
            case Graphics4::VertexTexCoord5:
            case Graphics4::VertexTexCoord6:
            case Graphics4::VertexTexCoord7:
                assert(size >= 1 && size <= 4);
                glClientActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(element.attribute - Graphics4::VertexTexCoord0));
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(size, type, myStride, reinterpret_cast<const void*>(internaloffset));
                break;

            default:
                break;
        }

        usedAttribsMask |= (1u << element.attribute);
        ++actualIndex;

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
		}
	}

    // Disable unused vertex attributes
	for (int attrib = Graphics4::VertexCoord; attrib <= Graphics4::VertexTexCoord7; ++attrib) {
        if ((usedAttribsMask & (1u << attrib)) == 0) {
            switch (attrib) {
                case Graphics4::VertexCoord:
                    glDisableClientState(GL_VERTEX_ARRAY);
                    break;

                case Graphics4::VertexNormal:
                    glDisableClientState(GL_NORMAL_ARRAY);
                    break;

                case Graphics4::VertexColor0:
                    glDisableClientState(GL_COLOR_ARRAY);
                    break;

                case Graphics4::VertexColor1:
                    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
                    break;

                case Graphics4::VertexTexCoord0:
                case Graphics4::VertexTexCoord1:
                case Graphics4::VertexTexCoord2:
                case Graphics4::VertexTexCoord3:
                case Graphics4::VertexTexCoord4:
                case Graphics4::VertexTexCoord5:
                case Graphics4::VertexTexCoord6:
                case Graphics4::VertexTexCoord7:
                    glClientActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(attrib - Graphics4::VertexTexCoord0));
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
