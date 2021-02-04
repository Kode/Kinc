#pragma once

#include <kinc/backend/event.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_event_impl_t impl;
} kinc_event_t;

KINC_FUNC void kinc_event_init(kinc_event_t *event, bool auto_clear);
KINC_FUNC void kinc_event_destroy(kinc_event_t *event);
KINC_FUNC void kinc_event_signal(kinc_event_t *event);
KINC_FUNC void kinc_event_wait(kinc_event_t *event);
KINC_FUNC bool kinc_event_try_to_wait(kinc_event_t *event, double seconds);
KINC_FUNC void kinc_event_reset(kinc_event_t *event);

#ifdef __cplusplus
}
#endif
