#include "pch.h"

#include <Kinc/Threads/Mutex.h>

#include <Windows.h>

#include <assert.h>

void Kinc_Mutex_Create(Kinc_Mutex *mutex) {
	assert(sizeof(RTL_CRITICAL_SECTION) == sizeof(Kinc_Microsoft_CriticalSection));
	InitializeCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void Kinc_Mutex_Destroy(Kinc_Mutex *mutex) {
	DeleteCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void Kinc_Mutex_Lock(Kinc_Mutex *mutex) {
	EnterCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

bool Kinc_Mutex_TryToLock(Kinc_Mutex *mutex) {
	return TryEnterCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void Kinc_Mutex_Unlock(Kinc_Mutex *mutex) {
	LeaveCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

bool Kinc_UberMutex_Create(Kinc_UberMutex *mutex, const char *name) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	mutex->impl.id = (void*)CreateMutexA(NULL, FALSE, name);
	HRESULT res = GetLastError();
	if (res && res != ERROR_ALREADY_EXISTS) {
		mutex->impl.id = NULL;
		assert(false);
		return false;
	}
	return true;
#else
	return false;
#endif
}

void Kinc_UberMutex_Destroy(Kinc_UberMutex *mutex) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	if (mutex->impl.id) {
		CloseHandle((HANDLE)mutex->impl.id);
		mutex->impl.id = NULL;
	}
#endif
}

void Kinc_UberMutex_Lock(Kinc_UberMutex *mutex) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	bool succ = WaitForSingleObject((HANDLE)mutex->impl.id, INFINITE) == WAIT_FAILED ? false : true;
	assert(succ);
#endif
}

void Kinc_UberMutex_Unlock(Kinc_UberMutex *mutex) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	bool succ = ReleaseMutex((HANDLE)mutex->impl.id) == FALSE ? false : true;
	assert(succ);
#endif
}
