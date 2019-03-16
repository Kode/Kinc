#include "pch.h"

#include "ThreadLocal.h"

#include <Kinc/Threads/ThreadLocal.h>

using namespace Kore;

void ThreadLocal::create() {
	Kinc_ThreadLocal_Create(&local);
}

void ThreadLocal::destroy() {
	Kinc_ThreadLocal_Destroy(&local);
}

void *ThreadLocal::get() {
	return Kinc_ThreadLocal_Get(&local);
}

void ThreadLocal::set(void *data) {
	Kinc_ThreadLocal_Set(&local, data);
}
