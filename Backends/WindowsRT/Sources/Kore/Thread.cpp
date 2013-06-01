#include "pch.h"
#include <Kore/Threads/Thread.h>
#include <Kore/Threads/Mutex.h>
#include <stdio.h>
#include <thread>
#include <windows.h>
#include <winuser.h>

using namespace Kore;

namespace {
	std::thread tt[MAX_THREADS];
	uint index = 0;
	Kore::Mutex mutex;
}

Kore::Thread* Kore::createAndRunThread(void (*thread)(void* param), void* param) {
	mutex.Lock();
	
	uint i = index++;
	//ktassert_d(i != 0xFFFFFFFF);
	std::thread* t = &tt[i];
	t->swap(std::thread(thread, param));

	mutex.Unlock();
	
	return (Kore::Thread*)t;
}

void Kore::waitForThreadStopThenFree(Kore::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	t->join();
	mutex.Lock();
	uint ti = ((uint)t - (uint)&tt[0]) / sizeof(::Thread);
	//ia.DeallocateIndex(ti);
	mutex.Unlock();
}

bool Kore::isThreadStoppedThenFree(Kore::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	return true;
}

#pragma warning(disable: 4996)
void Kore::threadsInit() {
	mutex.Create();
	//ia.Create(1);//SR_MAX_THREADS32);
}

void Kore::threadsQuit() {
	mutex.Free();
	//ia.Free();
}

void ThreadYield() {

}
