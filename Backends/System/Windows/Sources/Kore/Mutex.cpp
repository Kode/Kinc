#include "pch.h"

#include <Kore/Error.h>
#include <Kore/Threads/Mutex.h>

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

bool UberMutex::Create(const wchar_t* name) {
	id = (void*)CreateMutex(NULL, FALSE, name);
	HRESULT res = GetLastError();
	if (res && res != ERROR_ALREADY_EXISTS) {
		id = NULL;
		affirm(false);
		return false;
	}
	return true;
}

void UberMutex::Free() {
	if (id) {
		::CloseHandle((HANDLE)id);
		id = NULL;
	}
}

void UberMutex::Lock() {
	bool succ = WaitForSingleObject((HANDLE)id, INFINITE) == WAIT_FAILED ? false : true;
	affirm(succ);
}

void UberMutex::Unlock() {
	bool succ = ReleaseMutex((HANDLE)id) == FALSE ? false : true;
	affirm(succ);
}
