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
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	id = (void*)CreateMutex(NULL, FALSE, name);
	HRESULT res = GetLastError();
	if (res && res != ERROR_ALREADY_EXISTS) {
		id = NULL;
		affirm(false);
		return false;
	}
	return true;
#else
	return false;
#endif
}

void UberMutex::destroy() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	if (id) {
		::CloseHandle((HANDLE)id);
		id = NULL;
	}
#endif
}

void UberMutex::lock() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	bool succ = WaitForSingleObject((HANDLE)id, INFINITE) == WAIT_FAILED ? false : true;
	affirm(succ);
#endif
}

void UberMutex::unlock() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	bool succ = ReleaseMutex((HANDLE)id) == FALSE ? false : true;
	affirm(succ);
#endif
}
