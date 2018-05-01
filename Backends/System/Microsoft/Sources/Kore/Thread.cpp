#include "pch.h"
#include <stdio.h>
#include <string.h>

#include <Kore/Threads/Mutex.h>
#include <Kore/Threads/Thread.h>

#include <stdio.h>
#include <wchar.h>

using namespace Kore;

Thread* Kore::createAndRunThread(void (*thread)(void* param), void* param) {
	return nullptr;
}

void Kore::waitForThreadStopThenFree(Thread* sr) {

}

void Kore::threadsInit() {
	
}

void Kore::threadsQuit() {
	
}
