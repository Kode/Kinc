#pragma once

#include <kinc/backend/fiber.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_fiber_impl_t impl;
} kinc_fiber_t;

KINC_FUNC void kinc_fiber_init_current_thread(kinc_fiber_t *fiber);
KINC_FUNC void kinc_fiber_init(kinc_fiber_t *fiber, void (*func)(void *param), void *param);
KINC_FUNC void kinc_fiber_destroy(kinc_fiber_t *fiber);
KINC_FUNC void kinc_fiber_switch(kinc_fiber_t *fiber);

#ifdef __cplusplus
}
#endif
