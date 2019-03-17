#include "pch.h"

#include "Mutex.h"

using namespace Kore;

void Mutex::create() {
	Kinc_Mutex_Create(&mutex);
}

void Mutex::destroy() {
	Kinc_Mutex_Destroy(&mutex);
}

void Mutex::lock() {
	Kinc_Mutex_Lock(&mutex);
}

bool Mutex::tryToLock() {
	return Kinc_Mutex_TryToLock(&mutex);
}

void Mutex::unlock() {
	Kinc_Mutex_Unlock(&mutex);
}

bool UberMutex::create(const char *name) {
	return Kinc_UberMutex_Create(&mutex, name);
}

void UberMutex::destroy() {
	Kinc_UberMutex_Destroy(&mutex);
}

void UberMutex::lock() {
	Kinc_UberMutex_Lock(&mutex);
}

void UberMutex::unlock() {
	Kinc_UberMutex_Unlock(&mutex);
}
