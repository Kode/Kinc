#include "pch.h"

#include "Thread.h"

#include <kinc/threads/thread.h>

void Kore::threadsInit() {
	Kinc_Threads_Init();
}

void Kore::threadsQuit() {
	Kinc_Threads_Quit();
}

Kore::Thread *Kore::createAndRunThread(void(*func)(void *param), void *param) {
	Kore::Thread* thread = new Kore::Thread;
	Kinc_Thread_Create(&thread->thread, func, param);
	return thread;
}

void Kore::waitForThreadStopThenFree(Thread *thread) {
	Kinc_Thread_WaitAndDestroy(&thread->thread);
	delete thread;
}

bool Kore::isThreadStoppedThenFree(Thread *thread) {
	if (Kinc_Thread_TryToDestroy(&thread->thread)) {
		delete thread;
		return true;
	}
	else {
		return false;
	}
}

void Kore::threadSleep(int milliseconds) {
	Kinc_Thread_Sleep(milliseconds);
}
