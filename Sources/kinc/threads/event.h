#pragma once

#include <Kore/EventImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_event_impl_t impl;
} kinc_event_t;

void kinc_event_init(kinc_event_t *event);
void kinc_event_destroy(kinc_event_t *event);
void kinc_event_signal(kinc_event_t *event);
void kinc_event_wait(kinc_event_t *event);
bool kinc_event_try_to_wait(kinc_event_t *event, double seconds);
void kinc_event_reset(kinc_event_t *event);

#ifdef __cplusplus
}
#endif
