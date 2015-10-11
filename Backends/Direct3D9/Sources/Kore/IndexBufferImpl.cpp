#include "pch.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>
#include "Direct3D9.h"
#include "IndexBufferImpl.h"

using namespace Kore;

IndexBuffer* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	DWORD usage = 0;
#ifdef SYS_WINDOWS
	usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif
	affirm(device->CreateIndexBuffer(sizeof(int) * count, usage, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ib, 0));
}

IndexBuffer::~IndexBuffer() {
	ib->Release();
}

int* IndexBuffer::lock() {
	int* buffer;
	DWORD lockflags = 0;
#ifdef SYS_WINDOWS
	lockflags = D3DLOCK_DISCARD;
#endif
	int count2 = count();
#ifdef SYS_XBOX360
	count2 = 0;
#endif
	affirm(ib->Lock(0, count2 * 4, (void**)&buffer, lockflags));
	return buffer;
}

void IndexBuffer::unlock() {
	affirm(ib->Unlock());
}

void IndexBuffer::_set() {
	_current = this;
	affirm(device->SetIndices(ib));
}

int IndexBuffer::count() {
	return myCount;
}
