#include "pch.h"
#include <pthread.h>

#include <Kore/Threads/Mutex.h>

using namespace Kore;

void Mutex::Create() {
    //Kt::affirmD(sizeof(pthread_mutex) == sizeof(pthread_mutex_t));
	pthread_mutex_t *const ptm = (pthread_mutex_t*)&pthread_mutex[0];
	pthread_mutex_t temp = PTHREAD_MUTEX_INITIALIZER;
	*ptm = temp;
}

void Mutex::Free() {
	pthread_mutex_destroy((pthread_mutex_t*)&pthread_mutex[0]);
}

void Mutex::Lock() {
	int ret = pthread_mutex_lock((pthread_mutex_t*)&pthread_mutex[0]);
    //Kt::affirmD(ret == 0);
}

void Mutex::Unlock() {
	int ret = pthread_mutex_unlock((pthread_mutex_t*)&pthread_mutex[0]);
    //Kt::affirmD(ret == 0);
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
