#include "pch.h"

#include <kinc/threads/fiber.h>

#include <Windows.h>

LPFIBER_START_ROUTINE a;

void kinc_fiber_init_current_thread(kinc_fiber_t *fiber) {
	fiber->impl.fiber = ConvertThreadToFiber(NULL);
}

void kinc_fiber_init(kinc_fiber_t* fiber, void (*func)(void* param), void* param) {
	fiber->impl.fiber = CreateFiber(0, func, param);
}

void kinc_fiber_destroy(kinc_fiber_t* fiber) {
	DeleteFiber(fiber->impl.fiber);
	fiber->impl.fiber = NULL;
}

void kinc_fiber_switch(kinc_fiber_t* fiber) {
	SwitchToFiber(fiber->impl.fiber);
}
