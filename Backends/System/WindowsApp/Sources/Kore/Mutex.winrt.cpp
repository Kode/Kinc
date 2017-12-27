#include "pch.h"

#include <Kore/Threads/Mutex.h>

#include <Windows.h>

using namespace Kore;

void Mutex::create() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	InitializeCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::destroy() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	DeleteCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::lock() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	EnterCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::unlock() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	LeaveCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}
