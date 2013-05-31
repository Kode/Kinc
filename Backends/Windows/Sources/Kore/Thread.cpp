#include "pch.h"
#include <Kore/Threads/Thread.h>
#include <Kore/Threads/Mutex.h>
#include <stdio.h>

#include <Windows.h>
#include <WinUser.h>

using namespace Kore;

struct Thread {
	void *param;
	void (*thread)(void *param);
	void *handle;
};

::Thread         tt[MAX_THREADS];
//IndexAllocator ia;
Kore::Mutex       mutex;

namespace {
	int index = 0;
}

static DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	::Thread* t = (::Thread*)lpParameter;
	t->thread(t->param);
	return 0;
}

Kore::Thread* Kore::createAndRunThread(void (*thread)(void *param), void *param) {
	mutex.Lock();
	
	uint i = index++;///ia.AllocateIndex();
	//ktassert_d(i != 0xFFFFFFFF);
	
	::Thread* t = &tt[i];
	t->param  = param;
	t->thread = thread;
	t->handle = (void*)CreateThread(0, 65536, ThreadProc, t, 0, 0);
	
	mutex.Unlock();

	//lf("Kt::createAndRunThread(%p, %p) -> %p\n", thread, param, t);
	
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

void Kore::waitForThreadStopThenFree(Thread* sr) {
	//lf("Kt::createAndRunThread(%p): Start\n", sr);
	::Thread* t = (::Thread*)sr;
Again:;
	uint r = ::WaitForSingleObject(t->handle, 1000);
	if (r == WAIT_TIMEOUT) {
		goto Again;
	}
	#ifdef _TESTING
	ktassert(r == WAIT_OBJECT_0);
	#endif
	
	CloseHandle(t->handle);
	mutex.Lock();
	uint ti = ((uint)t - (uint)&tt[0]) / sizeof(::Thread);
	///ia.DeallocateIndex(ti);
	mutex.Unlock();
	//lf("Kt::createAndRunThread(%p): Done\n", t);
}

bool Kore::isThreadStoppedThenFree(Thread* sr) {
	::Thread* t = (::Thread*)sr;
	DWORD c;
	GetExitCodeThread(t->handle, &c);
	if (c != STILL_ACTIVE) {
		CloseHandle(t->handle);
		mutex.Lock();
		uint ti = ((uint)t - (uint)&tt[0]) / sizeof(::Thread);
		///ia.DeallocateIndex(ti);
		mutex.Unlock();
		//lf("Kt::isThreadStoppedThenFree(%p): Done\n", t);
		return true;
	}
	return false;
}

#pragma warning(disable: 4996)
void Kore::threadsInit() {
	mutex.Create();
	///ia.Create(1);//SR_MAX_THREADS32);
}

void Kore::threadsQuit() {
	mutex.Free();
	///ia.Free();
}

void ThreadYield() {
	SwitchToThread();
}
