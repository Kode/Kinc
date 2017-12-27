#include "pch.h"
#include <pthread.h>

#include <Kore/Threads/Mutex.h>

using namespace Kore;

void Mutex::create() {
	pthread_mutex_init(&pthread_mutex, nullptr);
}

void Mutex::destroy() {
	pthread_mutex_destroy(&pthread_mutex);
}

void Mutex::lock() {
	pthread_mutex_lock(&pthread_mutex);
}

void Mutex::unlock() {
	pthread_mutex_unlock(&pthread_mutex);
}

bool UberMutex::create(const wchar_t* name) {
	return false;
}

void UberMutex::destroy() {}

void UberMutex::lock() {
	// affirm(false);
}

void UberMutex::unlock() {
	// affirm(false);
}
