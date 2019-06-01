#include "pch.h"

#include <stdio.h>
#include <string.h>

#include <Foundation/Foundation.h>

#include <kinc/threads/mutex.h>
#include <kinc/threads/thread.h>

#include <pthread.h>
#include <stdio.h>
#include <wchar.h>

static void* ThreadProc(void* arg) {
	@autoreleasepool {
		Kinc_Thread *t = (Kinc_Thread*)arg;
		t->impl.thread(t->impl.param);
		pthread_exit(NULL);
		return NULL;
	}
}

void Kinc_Thread_Create(Kinc_Thread *t, void (*thread)(void* param), void* param) {
	t->impl.param = param;
	t->impl.thread = thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024 * 64);
	sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = 0;
	pthread_attr_setschedparam(&attr, &sp);
	int ret = pthread_create(&t->impl.pthread, &attr, &ThreadProc, t);
	// Kt::affirmD(ret == 0);
	pthread_attr_destroy(&attr);
}

void Kinc_Thread_WaitAndDestroy(Kinc_Thread *thread) {
	int ret;
	do {
		ret = pthread_join(thread->impl.pthread, NULL);
	} while (ret != 0);
}

bool Kinc_Thread_TryToDestroy(Kinc_Thread *thread) {
	return pthread_join(thread->impl.pthread, NULL) == 0;
}

void Kinc_Threads_Init() {
	
}

void Kinc_Threads_Quit() {
	
}
