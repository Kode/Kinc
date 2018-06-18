#include "pch.h"

#include <Kore/Threads/ThreadLocal.h>

using namespace Kore;

void ThreadLocal::create() {}

void ThreadLocal::destroy() {}

void* ThreadLocal::get() {
	return nullptr;
}

void ThreadLocal::set(void* data) {}
