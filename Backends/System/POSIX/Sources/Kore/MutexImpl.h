#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	pthread_mutex_t mutex;
} kinc_mutex_impl_t;
	
typedef struct {

} kinc_uber_mutex_impl_t;

#ifdef __cplusplus
}
#endif
