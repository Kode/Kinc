#include "pch.h"

#include <Kore/Threads/ThreadLocal.h>

#include <Windows.h>

using namespace Kore;

void ThreadLocal::create() {
	slot = TlsAlloc();
	TlsSetValue(slot, 0);
}

void ThreadLocal::destroy() {
	TlsFree(slot);
}

void* ThreadLocal::get() {
	return TlsGetValue(slot);
}

void ThreadLocal::set(void* data) {
	TlsSetValue(slot, data);
}
