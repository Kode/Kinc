#pragma once

#include <Kore/ThreadImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Kinc_ThreadImpl impl;
} Kinc_Thread;

void Kinc_Threads_Init();
void Kinc_Threads_Quit();

void Kinc_Thread_Create(Kinc_Thread *thread, void (*func)(void* param), void* param);
void Kinc_Thread_WaitAndDestroy(Kinc_Thread *thread);
bool Kinc_Thread_TryToDestroy(Kinc_Thread *thread);
void Kinc_Thread_Sleep(int milliseconds);

#ifdef __cplusplus
}
#endif
