#include "pch.h"

#include <Kinc/Threads/ThreadLocal.h>

#include <Windows.h>

void Kinc_ThreadLocal_Create(Kinc_ThreadLocal *local) {
	local->impl.slot = TlsAlloc();
	TlsSetValue(local->impl.slot, 0);
}

void Kinc_ThreadLocal_Destroy(Kinc_ThreadLocal *local) {
	TlsFree(local->impl.slot);
}

void *Kinc_ThreadLocal_Get(Kinc_ThreadLocal *local) {
	return TlsGetValue(local->impl.slot);
}

void Kinc_ThreadLocal_Set(Kinc_ThreadLocal *local, void *data) {
	TlsSetValue(local->impl.slot, data);
}
