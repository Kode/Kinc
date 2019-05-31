#include "pch.h"

#include <kinc/threads/thread.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

#if !defined(KORE_IOS) && !defined(KORE_MACOS)

static void* ThreadProc(void* arg) {
	Kinc_Thread *t = (Kinc_Thread*)arg;
	t->impl.thread(t->impl.param);
	pthread_exit(NULL);
	return NULL;
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

void Kinc_Threads_Init() {

}

void Kinc_Threads_Quit() {

}

#endif

void Kinc_Thread_Sleep(int milliseconds) {
	usleep(1000 * (useconds_t)milliseconds);
}
