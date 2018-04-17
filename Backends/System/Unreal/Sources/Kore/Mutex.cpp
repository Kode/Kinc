#include "pch.h"

#include <Kore/Error.h>
#include <Kore/Threads/Mutex.h>

using namespace Kore;

void Mutex::create() {}

void Mutex::destroy() {}

void Mutex::lock() {}

void Mutex::unlock() {}

bool UberMutex::create(const wchar_t* name) {
	return false;
}

void UberMutex::destroy() {}

void UberMutex::lock() {}

void UberMutex::unlock() {}
