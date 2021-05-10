#include "Mutex.h"

using namespace Kore;

void Mutex::create() {
	kinc_mutex_init(&mutex);
}

void Mutex::destroy() {
	kinc_mutex_destroy(&mutex);
}

void Mutex::lock() {
	kinc_mutex_lock(&mutex);
}

bool Mutex::tryToLock() {
	return kinc_mutex_try_to_lock(&mutex);
}

void Mutex::unlock() {
	kinc_mutex_unlock(&mutex);
}

bool UberMutex::create(const char *name) {
	return kinc_uber_mutex_init(&mutex, name);
}

void UberMutex::destroy() {
	kinc_uber_mutex_destroy(&mutex);
}

void UberMutex::lock() {
	kinc_uber_mutex_lock(&mutex);
}

void UberMutex::unlock() {
	kinc_uber_mutex_unlock(&mutex);
}
