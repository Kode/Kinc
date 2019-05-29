#include "pch.h"

#include "Mouse.h"

#include <Kinc/Window.h>

#include <memory.h>

void (*Kinc_Mouse_PressCallback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/) = NULL;
void (*Kinc_Mouse_ReleaseCallback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/) = NULL;
void (*Kinc_Mouse_MoveCallback)(int /*window*/, int /*x*/, int /*y*/, int /*movementX*/, int /*movementY*/) = NULL;
void (*Kinc_Mouse_ScrollCallback)(int /*window*/, int /*delta*/) = NULL;
void (*Kinc_Mouse_EnterWindowCallback)(int /*window*/) = NULL;
void (*Kinc_Mouse_LeaveWindowCallback)(int /*window*/) = NULL;

void Kinc_Internal_Mouse_TriggerRelease(int window, int button, int x, int y) {
	if (Kinc_Mouse_ReleaseCallback != NULL) {
		Kinc_Mouse_ReleaseCallback(window, button, x, y);
	}
}

void Kinc_Internal_Mouse_TriggerScroll(int window, int delta) {
	if (Kinc_Mouse_ScrollCallback != NULL) {
		Kinc_Mouse_ScrollCallback(window, delta);
	}
}

void Kinc_Internal_Mouse_TriggerEnterWindow(int window) {
	if (Kinc_Mouse_EnterWindowCallback != NULL) {
		Kinc_Mouse_EnterWindowCallback(window);
	}
}

void Kinc_Internal_Mouse_TriggerLeaveWindow(int window) {
	if (Kinc_Mouse_LeaveWindowCallback != NULL) {
		Kinc_Mouse_LeaveWindowCallback(window);
	}
}

void Kinc_Internal_Mouse_WindowActivated(int window) {
	if (Kinc_Mouse_IsLocked(window)) {
		Kinc_Internal_Mouse_Lock(window);
	}
}
void Kinc_Internal_Mouse_WindowDeactivated(int window) {
	if (Kinc_Mouse_IsLocked(window)) {
		Kinc_Internal_Mouse_Unlock(window);
	}
}

// TODO: handle state per window
static bool moved = false;
static bool locked = false;
static int preLockX = 0;
static int preLockY = 0;
static int centerX = 0;
static int centerY = 0;
static int lastX = 0;
static int lastY = 0;

void Kinc_Internal_Mouse_TriggerPress(int window, int button, int x, int y) {
	lastX = x;
	lastY = y;
	if (Kinc_Mouse_PressCallback != NULL) {
		Kinc_Mouse_PressCallback(window, button, x, y);
	}
}

void Kinc_Internal_Mouse_TriggerMove(int window, int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (Kinc_Mouse_IsLocked(window)) {
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0) {
			Kinc_Mouse_SetPosition(window, centerX, centerY);
			x = centerX;
			y = centerY;
		}
	}
	else if (moved) {
		movementX = x - lastX;
		movementY = y - lastY;
	}
	moved = true;

	lastX = x;
	lastY = y;
	if (Kinc_Mouse_MoveCallback != NULL && (movementX != 0 || movementY != 0)) {
		Kinc_Mouse_MoveCallback(window, x, y, movementX, movementY);
	}
}

bool Kinc_Mouse_IsLocked(int window) {
	return locked;
}

void Kinc_Mouse_Lock(int window) {
	if (!Kinc_Mouse_CanLock(window)) {
		return;
	}
	locked = true;
	Kinc_Internal_Mouse_Lock(window);
	Kinc_Mouse_GetPosition(window, &preLockX, &preLockY);
	centerX = Kinc_WindowWidth(window) / 2;
	centerY = Kinc_WindowHeight(window) / 2;
	Kinc_Mouse_SetPosition(window, centerX, centerY);
}

void Kinc_Mouse_Unlock(int window) {
	if (!Kinc_Mouse_CanLock(window)) {
		return;
	}
	moved = false;
	locked = false;
	Kinc_Internal_Mouse_Unlock(window);
	Kinc_Mouse_SetPosition(window, preLockX, preLockY);
}
