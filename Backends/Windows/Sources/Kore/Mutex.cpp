#include "pch.h"
#include <Kore/Threads/Mutex.h>
#include <Kore/Error.h>
#include <Windows.h>

using namespace Kore;

void Mutex::Create() {
	InitializeCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::Free() {
	DeleteCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::Lock() {
	EnterCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::Unlock() {
	LeaveCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UberMutex::Create(const wchar_t *name) {
	#ifdef SYS_WINDOWS
	/* Create the mutex, with initial value signaled */
	(HANDLE&)id = ::CreateMutex(NULL, FALSE, name);
	if (HRESULT res = GetLastError()) {
		if (res != ERROR_ALREADY_EXISTS) {
			id = NULL;
			affirm(false);
			return false;
		}
	}
	return true;
	#else
	return false;
	#endif
}

void UberMutex::Free() {
	#ifdef SYS_WINDOWS
	if (id) {
		::CloseHandle((HANDLE)id);
		id = NULL;
	}
	#else
	#endif
}

void UberMutex::Lock() {
	#ifdef SYS_WINDOWS
	bool succ = ( ::WaitForSingleObject((HANDLE)id, INFINITE) == WAIT_FAILED ) ? (false): (true);
	affirm(succ);
	#else
	affirm(false);
	#endif
}

/* Unlock the mutex */
void UberMutex::Unlock() {
	#ifdef SYS_WINDOWS
	bool succ = ( ::ReleaseMutex((HANDLE)id) == FALSE ) ? (false): (true);
	affirm(succ);
	#else
	affirm(false);
	#endif
}
