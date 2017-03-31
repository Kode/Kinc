#include "pch.h"

#include "Direct3D9.h"
#include "IndexBufferImpl.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/WinError.h>

using namespace Kore;

Graphics4::IndexBuffer* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {}

Graphics4::IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	DWORD usage = 0;
#ifdef KORE_WINDOWS
	usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif
	affirm(device->CreateIndexBuffer(sizeof(int) * count, usage, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ib, 0));
}

Graphics4::IndexBuffer::~IndexBuffer() {
	ib->Release();
}

int* Graphics4::IndexBuffer::lock() {
	int* buffer;
	DWORD lockflags = 0;
#ifdef KORE_WINDOWS
	lockflags = D3DLOCK_DISCARD;
#endif
	int count2 = count();
	affirm(ib->Lock(0, count2 * 4, (void**)&buffer, lockflags));
	return buffer;
}

void Graphics4::IndexBuffer::unlock() {
	affirm(ib->Unlock());
}

void Graphics4::IndexBuffer::_set() {
	_current = this;
	affirm(device->SetIndices(ib));
}

int Graphics4::IndexBuffer::count() {
	return myCount;
}
