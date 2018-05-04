#include "pch.h"

#include <Kore/Threads/Mutex.h>
#include <Kore/Threads/Thread.h>

#include <stdio.h>

#include <Windows.h>

#include <WinUser.h>

using namespace Kore;

struct ThreadData {
	void* param;
	void(*thread)(void* param);
	HANDLE handle;
};

static DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	ThreadData* data = (ThreadData*)lpParameter;
	data->thread(data->param);
	return 0;
}

Kore::Thread* Kore::createAndRunThread(void(*thread)(void* param), void* param) {
	ThreadData* data = new ThreadData;
	data->param = param;
	data->thread = thread;
	HANDLE handle = CreateThread(0, 65536, ThreadProc, data, 0, 0);
	return (Thread*)data;
}

void Kore::waitForThreadStopThenFree(Thread* thread) {
	ThreadData* data = (ThreadData*)thread;
	uint wait;
	do {
		wait = WaitForSingleObject(data->handle, 1000);
	} while (wait == WAIT_TIMEOUT);
	CloseHandle(data->handle);
	delete data;
}

bool Kore::isThreadStoppedThenFree(Thread* thread) {
	ThreadData* data = (ThreadData*)thread;
	DWORD code;
	GetExitCodeThread(data->handle, &code);
	if (code != STILL_ACTIVE) {
		CloseHandle(data->handle);
		delete data;
		return true;
	}
	return false;
}

void Kore::threadsInit() {}

void Kore::threadsQuit() {}
