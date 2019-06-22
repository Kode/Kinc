#include "pch.h"

#include "ThreadLocal.h"

#include <kinc/threads/threadlocal.h>

using namespace Kore;

void ThreadLocal::create() {
	kinc_thread_local_init(&local);
}

void ThreadLocal::destroy() {
	kinc_thread_local_destroy(&local);
}

void *ThreadLocal::get() {
	return kinc_thread_local_get(&local);
}

void ThreadLocal::set(void *data) {
	kinc_thread_local_set(&local, data);
}
