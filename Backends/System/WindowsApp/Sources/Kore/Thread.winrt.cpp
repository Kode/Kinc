#include "pch.h"
#include <Kore/Threads/Mutex.h>
#include <Kore/Threads/Thread.h>
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
	mutex.lock();

	uint i = index++;
	// ktassert_d(i != 0xFFFFFFFF);
	std::thread* t = &tt[i];
	t->swap(std::thread(thread, param));

	mutex.unlock();

	return (Kore::Thread*)t;
}

void Kore::waitForThreadStopThenFree(Kore::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	t->join();
	mutex.lock();
	uint ti = ((uint)t - (uint)&tt[0]) / sizeof(::Thread);
	// ia.DeallocateIndex(ti);
	mutex.unlock();
}

bool Kore::isThreadStoppedThenFree(Kore::Thread* sr) {
	std::thread* t = (std::thread*)sr;
	return true;
}

#pragma warning(disable : 4996)
void Kore::threadsInit() {
	mutex.create();
	// ia.Create(1);//SR_MAX_THREADS32);
}

void Kore::threadsQuit() {
	mutex.destroy();
	// ia.Free();
}

void ThreadYield() {}
