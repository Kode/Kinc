#pragma once

#include <Kore/ThreadLocalImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Kinc_ThreadLocalImpl impl;
} Kinc_ThreadLocal;

void Kinc_ThreadLocal_Create(Kinc_ThreadLocal *local);
void Kinc_ThreadLocal_Destroy(Kinc_ThreadLocal *local);
void *Kinc_ThreadLocal_Get(Kinc_ThreadLocal *local);
void Kinc_ThreadLocal_Set(Kinc_ThreadLocal *local, void *data);

#ifdef __cplusplus
}
#endif
