#include "pch.h"

#include <Kore/Threads/Mutex.h>
#include <Kore/Threads/Thread.h>

#include <stdio.h>

#include <Windows.h>

#include <WinUser.h>

using namespace Kore;

struct ThreadData {
	void* param;
	void (*thread)(void* param);
	HANDLE handle;
};

static DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	ThreadData* data = (ThreadData*)lpParameter;
	data->thread(data->param);
	return 0;
}

Kore::Thread* Kore::createAndRunThread(void (*thread)(void* param), void* param) {
	ThreadData* data = new ThreadData;
	data->param = param;
	data->thread = thread;
	data->handle = CreateThread(0, 65536, ThreadProc, data, 0, 0);

	return (Thread*)data;
}

void Kore::waitForThreadStopThenFree(Thread* thread) {
	ThreadData* data = (ThreadData*)thread;
Again:;
	uint r = ::WaitForSingleObject(data->handle, 1000);
	if (r == WAIT_TIMEOUT) {
		goto Again;
	}

	CloseHandle(data->handle);
}

bool Kore::isThreadStoppedThenFree(Thread* thread) {
	ThreadData* data = (ThreadData*)thread;
	DWORD c;
	GetExitCodeThread(data->handle, &c);
	if (c != STILL_ACTIVE) {
		CloseHandle(data->handle);
		return true;
	}
	return false;
}

void Kore::threadsInit() {

}

void Kore::threadsQuit() {

}
