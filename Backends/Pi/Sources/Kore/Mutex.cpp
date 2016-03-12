#include "pch.h"
#include <pthread.h>

#include <Kore/Threads/Mutex.h>

using namespace Kore;

void Mutex::Create() {
	pthread_mutex_init(&pthread_mutex, nullptr);
}

void Mutex::Free() {
	pthread_mutex_destroy(&pthread_mutex);
}

void Mutex::Lock() {
	pthread_mutex_lock(&pthread_mutex);
}

void Mutex::Unlock() {
	pthread_mutex_unlock(&pthread_mutex);
}

bool UberMutex::Create(const wchar_t *name) {
	return false;
}

void UberMutex::Free() {

}

void UberMutex::Lock() {
	//affirm(false);
}

void UberMutex::Unlock() {
	//affirm(false);
}
