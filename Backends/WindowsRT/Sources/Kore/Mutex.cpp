#include "pch.h"
#include <Kore/Threads/Mutex.h>
#include <Windows.h>

using namespace Kore;

void Mutex::Create() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	InitializeCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::Free() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	DeleteCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::Lock() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	EnterCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}

void Mutex::Unlock() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	LeaveCriticalSection((CRITICAL_SECTION*)&criticalSection);
#endif
}