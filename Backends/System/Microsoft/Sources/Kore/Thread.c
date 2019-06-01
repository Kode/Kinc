#include "pch.h"

#include <kinc/threads/semaphore.h>
#include <kinc/threads/thread.h>

#include <stdio.h>

#include <Windows.h>

#include <WinUser.h>

static void (*next_thread_func)(void*);
static void *next_thread_param;
static kinc_semaphore_t ready;

void kinc_threads_init() {
	kinc_semaphore_init(&ready, 0, 1);
}

void kinc_threads_quit() {
	kinc_semaphore_destroy(&ready);
}

static DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	void (*func)(void *) = next_thread_func;
	void *param = next_thread_param;
	kinc_semaphore_release(&ready, 1);
	func(param);
	return 0;
}

void kinc_thread_init(kinc_thread_t *thread, void (*func)(void *param), void *param) {
	kinc_semaphore_acquire(&ready);
	next_thread_func = func;
	next_thread_param = param;
	thread->impl.func = func;
	thread->impl.param = param;
	thread->impl.handle = CreateThread(0, 65536, ThreadProc, NULL, 0, 0);
}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {
	unsigned wait;
	do {
		wait = WaitForSingleObject(thread->impl.handle, 1000);
	} while (wait == WAIT_TIMEOUT);
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
