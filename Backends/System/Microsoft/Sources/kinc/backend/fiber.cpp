#include "pch.h"

#include <kinc/threads/fiber.h>

#include <Windows.h>

VOID WINAPI fiber_func(LPVOID param) {
#ifndef KORE_WINDOWSAPP
	kinc_fiber_t *fiber = (kinc_fiber_t *)param;
	fiber->impl.func(fiber->impl.param);
#endif
}

void kinc_fiber_init_current_thread(kinc_fiber_t *fiber) {
#ifndef KORE_WINDOWSAPP
	fiber->impl.fiber = ConvertThreadToFiber(NULL);
#endif
}

void kinc_fiber_init(kinc_fiber_t* fiber, void (*func)(void* param), void* param) {
#ifndef KORE_WINDOWSAPP
	fiber->impl.func = func;
	fiber->impl.param = param;
	fiber->impl.fiber = CreateFiber(0, fiber_func, fiber);
#endif
}

void kinc_fiber_destroy(kinc_fiber_t* fiber) {
#ifndef KORE_WINDOWSAPP
	DeleteFiber(fiber->impl.fiber);
	fiber->impl.fiber = NULL;
#endif
}

void kinc_fiber_switch(kinc_fiber_t* fiber) {
#ifndef KORE_WINDOWSAPP
	SwitchToFiber(fiber->impl.fiber);
#endif
}
