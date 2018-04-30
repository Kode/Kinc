#include "pch.h"

#include <Kore/Error.h>
#include <Kore/Threads/Mutex.h>

#include <Windows.h>

#include <assert.h>

using namespace Kore;

void Mutex::create() {
	assert(sizeof(RTL_CRITICAL_SECTION) == sizeof(Mutex::CriticalSection));
	InitializeCriticalSection((LPCRITICAL_SECTION)&criticalSection);
}

void Mutex::destroy() {
	DeleteCriticalSection((LPCRITICAL_SECTION)&criticalSection);
}

void Mutex::lock() {
	EnterCriticalSection((LPCRITICAL_SECTION)&criticalSection);
}

bool Mutex::tryToLock() {
	return TryEnterCriticalSection((LPCRITICAL_SECTION)&criticalSection);
}

void Mutex::unlock() {
	LeaveCriticalSection((LPCRITICAL_SECTION)&criticalSection);
}

bool UberMutex::create(const wchar_t* name) {
	return false;
}

void UberMutex::destroy() {
	
}

void UberMutex::lock() {
	
}

void UberMutex::unlock() {
	
}
