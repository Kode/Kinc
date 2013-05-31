#include "pch.h"
#include <stdio.h>
#include <string.h>

#include <Kore/Threads/Thread.h>
#include <Kore/Threads/Mutex.h>
#include <pthread.h>
#include <Foundation/Foundation.h>
#include <stdio.h>
#include <wchar.h>

using namespace Kore;

struct IOS_Thread {
	void *param;
	void (*thread)(void *param);
	pthread_t pthread;
};
IOS_Thread         tt[MAX_THREADS];
//IndexAllocator ia;
uint threadindex = 0;
Mutex       mutex;

static void* ThreadProc(void *arg) {
	@autoreleasepool {
		IOS_Thread *t = (IOS_Thread*)arg;
		t->thread(t->param);
		pthread_exit(NULL);
	}
}

Thread* Kore::createAndRunThread(void (*thread)(void *param), void *param) {
	mutex.Lock();
	
	uint i = threadindex++;//ia.AllocateIndex();
	//ktassert_d(i != 0xFFFFFFFF);
	
	IOS_Thread *t = &tt[i];
	t->param  = param;
	t->thread = thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*64);
	sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = 0;
	pthread_attr_setschedparam(&attr, &sp);
	int ret = pthread_create(&t->pthread, &attr, &ThreadProc, t);
    //Kt::affirmD(ret == 0);
	pthread_attr_destroy(&attr);
	
	mutex.Unlock();
	
	return (Thread*)t;
}

/*
void SR_StopThread(SR_Thread *sr) {
	mutex.Lock();
	Thread *t = (Thread*)sr;
	CloseHandle(t->handle);
	mutex.Unlock();
}
*/

void Kore::waitForThreadStopThenFree(Thread *sr) {
	mutex.Lock();
	IOS_Thread *t = (IOS_Thread*)sr;
Again:;
	int ret = pthread_join(t->pthread, NULL);
	if (ret != 0) goto Again;
	mutex.Unlock();
	int ti = static_cast<int>(((upint)t - (upint)&tt[0]) / sizeof(IOS_Thread));
	//ia.DeallocateIndex(ti);
}

void Kore::threadsInit() {
	mutex.Create();
	//ia.Create(1);//SR_MAX_THREADS32);
}

void Kore::threadsQuit() {
	mutex.Free();
	//ia.Free();
}
