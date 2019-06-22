#pragma once

#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	sem_t semaphore;
} kinc_semaphore_impl_t;

#ifdef __cplusplus
}
#endif
