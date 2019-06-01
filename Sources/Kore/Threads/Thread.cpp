#include "pch.h"

#include "Thread.h"

#include <kinc/threads/thread.h>

void Kore::threadsInit() {
	kinc_threads_init();
}

void Kore::threadsQuit() {
	kinc_threads_quit();
}

Kore::Thread *Kore::createAndRunThread(void(*func)(void *param), void *param) {
	Kore::Thread* thread = new Kore::Thread;
	kinc_thread_init(&thread->thread, func, param);
	return thread;
}

void Kore::waitForThreadStopThenFree(Thread *thread) {
	kinc_thread_wait_and_destroy(&thread->thread);
	delete thread;
}

bool Kore::isThreadStoppedThenFree(Thread *thread) {
	if (kinc_thread_try_to_destroy(&thread->thread)) {
		delete thread;
		return true;
	}
	else {
		return false;
	}
}

void Kore::threadSleep(int milliseconds) {
	kinc_thread_sleep(milliseconds);
}
