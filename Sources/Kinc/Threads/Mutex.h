#pragma once

#include <Kore/MutexImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Kinc_MutexImpl impl;
} Kinc_Mutex;

void Kinc_Mutex_Create(Kinc_Mutex *mutex);
void Kinc_Mutex_Destroy(Kinc_Mutex *mutex);
void Kinc_Mutex_Lock(Kinc_Mutex *mutex);
bool Kinc_Mutex_TryToLock(Kinc_Mutex *mutex);
void Kinc_Mutex_Unlock(Kinc_Mutex *mutex);

typedef struct {
	Kinc_UberMutexImpl impl;
} Kinc_UberMutex;

bool Kinc_UberMutex_Create(Kinc_UberMutex *mutex, const char *name);
void Kinc_UberMutex_Destroy(Kinc_UberMutex *mutex);
void Kinc_UberMutex_Lock(Kinc_UberMutex *mutex);
void Kinc_UberMutex_Unlock(Kinc_UberMutex *mutex);

#ifdef __cplusplus
}
#endif
