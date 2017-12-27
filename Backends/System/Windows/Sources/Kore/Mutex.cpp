#include "pch.h"

#include <Kore/Error.h>
#include <Kore/Threads/Mutex.h>

#include <Windows.h>

using namespace Kore;

void Mutex::create() {
	InitializeCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::destroy() {
	DeleteCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::lock() {
	EnterCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

void Mutex::unlock() {
	LeaveCriticalSection((CRITICAL_SECTION*)&criticalSection);
}

bool UberMutex::create(const wchar_t* name) {
	id = (void*)CreateMutex(NULL, FALSE, name);
	HRESULT res = GetLastError();
	if (res && res != ERROR_ALREADY_EXISTS) {
		id = NULL;
		affirm(false);
		return false;
	}
	return true;
}

void UberMutex::destroy() {
	if (id) {
		::CloseHandle((HANDLE)id);
		id = NULL;
	}
}

void UberMutex::lock() {
	bool succ = WaitForSingleObject((HANDLE)id, INFINITE) == WAIT_FAILED ? false : true;
	affirm(succ);
}

void UberMutex::unlock() {
	bool succ = ReleaseMutex((HANDLE)id) == FALSE ? false : true;
	affirm(succ);
}
