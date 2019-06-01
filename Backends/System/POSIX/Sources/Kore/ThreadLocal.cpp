#include "pch.h"

#include <kinc/threads/threadlocal.h>

void Kinc_ThreadLocal_Create(Kinc_ThreadLocal *local) {}

void Kinc_ThreadLocal_Destroy(Kinc_ThreadLocal *local) {}

void* Kinc_ThreadLocal_Get(Kinc_ThreadLocal *local) {
	return nullptr;
}

void Kinc_ThreadLocal_Set(Kinc_ThreadLocal *local, void* data) {}
