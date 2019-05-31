#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* param;
    void (*thread)(void* param);
    pthread_t pthread;
} Kinc_ThreadImpl;

#ifdef __cplusplus
}
#endif
