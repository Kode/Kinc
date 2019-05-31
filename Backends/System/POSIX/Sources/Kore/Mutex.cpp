#include "pch.h"

#include <kinc/threads/mutex.h>

#include <pthread.h>

using namespace Kore;

void Kinc_Mutex_Create(Kinc_Mutex *mutex) {
	pthread_mutex_init(&mutex->impl.mutex, NULL);
}

void Kinc_Mutex_Destroy(Kinc_Mutex *mutex) {
	pthread_mutex_destroy(&mutex->impl.mutex);
}

void Kinc_Mutex_Lock(Kinc_Mutex *mutex) {
	pthread_mutex_lock(&mutex->impl.mutex);
}

void Kinc_Mutex_Unlock(Kinc_Mutex *mutex) {
	pthread_mutex_unlock(&mutex->impl.mutex);
}

bool Kinc_UberMutex_Create(Kinc_UberMutex *mutex, const wchar_t* name) {
	return false;
}

void Kinc_UberMutex_Destroy(Kinc_UberMutex *mutex) {}

void Kinc_UberMutex_Lock(Kinc_UberMutex *mutex) {
	// affirm(false);
}

void Kinc_UberMutex_Unlock(Kinc_UberMutex *mutex) {
	// affirm(false);
}
