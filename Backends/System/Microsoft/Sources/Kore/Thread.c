#include "pch.h"

#include <kinc/threads/thread.h>

#include <stdio.h>

#include <Windows.h>

void kinc_threads_init() {
	
}

void kinc_threads_quit() {
	
}

static DWORD WINAPI ThreadProc(LPVOID param) {
	kinc_thread_t *thread = param;
	thread->impl.func(thread->impl.param);
	return 0;
}

void kinc_thread_init(kinc_thread_t *thread, void (*func)(void *param), void *param) {
	thread->impl.func = func;
	thread->impl.param = param;
	thread->impl.handle = CreateThread(0, 65536, ThreadProc, thread, 0, 0);
}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {
	WaitForSingleObject(thread->impl.handle, INFINITE);
	CloseHandle(thread->impl.handle);
}

bool kinc_thread_try_to_destroy(kinc_thread_t *thread) {
	DWORD code;
	GetExitCodeThread(thread->impl.handle, &code);
	if (code != STILL_ACTIVE) {
		CloseHandle(thread->impl.handle);
		return true;
	}
	return false;
}

void kinc_thread_sleep(int milliseconds) {
	Sleep(milliseconds);
}
