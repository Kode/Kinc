#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t condvar;
} kinc_event_impl_t;
	
#ifdef __cplusplus
}
#endif
