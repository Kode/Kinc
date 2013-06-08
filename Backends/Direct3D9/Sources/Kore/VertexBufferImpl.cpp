#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>
#include "Direct3D9.h"

using namespace Kore;

VertexBuffer* VertexBufferImpl::_current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count) : myCount(count) {

}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure) : VertexBufferImpl(count) {
	DWORD usage = D3DUSAGE_WRITEONLY;
#ifdef SYS_WINDOWS
	usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		switch (structure.elements[i].data) {
		case Float2VertexData:
			myStride += 4 * 2;
			break;
		case Float3VertexData:
			myStride += 4 * 3;
			break;
		case Float4VertexData:
			myStride += 4 * 4;
			break;
		case ColorVertexData:
			myStride += 4;
			break;
		}
	}
	
	affirm(device->CreateVertexBuffer(stride() * count, usage, 0, D3DPOOL_DEFAULT, &vb, 0));
}

VertexBuffer::~VertexBuffer() {
	vb->Release();
}

float* VertexBuffer::lock() {
	return lock(0, count());
}

float* VertexBuffer::lock(int start, int count) {
	float* vertices;
	unset();
#ifdef SYS_XBOX360
	vb->Lock(start, 0, (void**)&vertices, 0);
#else
	affirm(vb->Lock(start, count * stride(), (void**)&vertices, D3DLOCK_DISCARD));
#endif
	return vertices;
}

void VertexBuffer::unlock() {
	affirm(vb->Unlock());
}

void VertexBuffer::set() {
	_current = this;
	affirm(device->SetStreamSource(0, vb, 0, stride()));
}

void VertexBufferImpl::unset() {
	if (_current == (VertexBuffer*)this) {
		affirm(device->SetStreamSource(0, nullptr, 0, 0));
		_current = nullptr;
	}
}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}
