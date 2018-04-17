#include "pch.h"

#include <Kore/Threads/Mutex.h>
#include <Kore/Threads/Thread.h>

#include <stdio.h>

using namespace Kore;

Kore::Thread* Kore::createAndRunThread(void (*thread)(void* param), void* param) {
	return nullptr;
}

void Kore::waitForThreadStopThenFree(Thread* sr) {}

bool Kore::isThreadStoppedThenFree(Thread* sr) {
	return false;
}

void Kore::threadsInit() {}

void Kore::threadsQuit() {}

void ThreadYield() {}
