#include "pch.h"

#include <Kinc/Threads/Semaphore.h>
#include <Kinc/Threads/Thread.h>

#include <stdio.h>

#include <Windows.h>

#include <WinUser.h>

static void (*next_thread_func)(void*);
static void *next_thread_param;
static Kinc_Semaphore ready;

void Kinc_Threads_Init() {
	Kinc_Semaphore_Create(&ready, 0, 1);
}

void Kinc_Threads_Quit() {
	Kinc_Semaphore_Destroy(&ready);
}

static DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	void (*func)(void *) = next_thread_func;
	void *param = next_thread_param;
	Kinc_Semaphore_Release(&ready, 1);
	func(param);
	return 0;
}

void Kinc_Thread_Create(Kinc_Thread *thread, void (*func)(void *param), void *param) {
	Kinc_Semaphore_Acquire(&ready);
	next_thread_func = func;
	next_thread_param = param;
	thread->impl.func = func;
	thread->impl.param = param;
	thread->impl.handle = CreateThread(0, 65536, ThreadProc, NULL, 0, 0);
}

void Kinc_Thread_WaitAndDestroy(Kinc_Thread *thread) {
	unsigned wait;
	do {
		wait = WaitForSingleObject(thread->impl.handle, 1000);
	} while (wait == WAIT_TIMEOUT);
	CloseHandle(thread->impl.handle);
}

bool Kinc_Thread_TryToDestroy(Kinc_Thread *thread) {
	DWORD code;
	GetExitCodeThread(thread->impl.handle, &code);
	if (code != STILL_ACTIVE) {
		CloseHandle(thread->impl.handle);
		return true;
	}
	return false;
}

void Kinc_Thread_Sleep(int milliseconds) {
	Sleep(milliseconds);
}
