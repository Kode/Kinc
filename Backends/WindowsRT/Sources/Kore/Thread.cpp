#include "stdafx.h"
#include <Kt/Thread.h>
#include <Kt/Mutex.h>
#include <stdio.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#include <thread>
#endif

#include <windows.h>
#include <winuser.h>

using namespace Kt;

namespace {
	std::thread    tt[MAX_THREADS];
	uint index = 0;
	Kt::Mutex      mutex;
}

Kt::Thread* Kt::createAndRunThread(void (*thread)(void *param), void *param) {
	mutex.Lock();
	
	Kt::uint i = index++;
	//ktassert_d(i != 0xFFFFFFFF);
	std::thread* t = &tt[i];
	t->swap(std::thread(thread, param));

	mutex.Unlock();
	
	return (Kt::Thread*)t;
}

/*
void SR_StopThread(SR_Thread *sr) {
	mutex.Lock();
	Thread *t = (Thread*)sr;
	CloseHandle(t->handle);
	mutex.Unlock();
}
*/

void Kt::waitForThreadStopThenFree(Kt::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	t->join();
	mutex.Lock();
	Kt::uint ti = ((Kt::uint)t - (Kt::uint)&tt[0]) / sizeof(::Thread);
	//ia.DeallocateIndex(ti);
	mutex.Unlock();
}

bool Kt::isThreadStoppedThenFree(Kt::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	return true;
}

#pragma warning(disable: 4996)
void Kt::threadsInit() {
	mutex.Create();
	//ia.Create(1);//SR_MAX_THREADS32);
}

void Kt::threadsQuit() {
	mutex.Free();
	//ia.Free();
}

void ThreadYield() {

}