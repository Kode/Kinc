#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *DebugInfo;
	long LockCount;
	long RecursionCount;
	void *OwningThread;
	void *LockSemaphore;
	unsigned long __w64 SpinCount;
} Kinc_Microsoft_CriticalSection;

typedef struct {
	Kinc_Microsoft_CriticalSection criticalSection;
} Kinc_MutexImpl;

typedef struct {
	void *id;
} Kinc_UberMutexImpl;

#ifdef __cplusplus
}
#endif
