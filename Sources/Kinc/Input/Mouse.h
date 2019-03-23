#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Kinc_Mouse_PressCallback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
extern void (*Kinc_Mouse_ReleaseCallback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
extern void (*Kinc_Mouse_MoveCallback)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/);
extern void (*Kinc_Mouse_ScrollCallback)(int /*window*/, int /*delta*/);
extern void (*Kinc_Mouse_EnterWindowCallback)(int /*window*/);
extern void (*Kinc_Mouse_LeaveWindowCallback)(int /*window*/);

bool Kinc_Mouse_CanLock(int window);
bool Kinc_Mouse_IsLocked(int window);
void Kinc_Mouse_Lock(int window);
void Kinc_Mouse_Unlock(int window);

void Kinc_Mouse_Show();
void Kinc_Mouse_Hide();
void Kinc_Mouse_SetPosition(int window, int x, int y);
void Kinc_Mouse_GetPosition(int window, int *x, int *y);

void Kinc_Internal_Mouse_TriggerPress(int window, int button, int x, int y);
void Kinc_Internal_Mouse_TriggerRelease(int window, int button, int x, int y);
void Kinc_Internal_Mouse_TriggerMove(int window, int x, int y);
void Kinc_Internal_Mouse_TriggerScroll(int window, int delta);
void Kinc_Internal_Mouse_TriggerEnterWindow(int window);
void Kinc_Internal_Mouse_TriggerLeaveWindow(int window);
void Kinc_Internal_Mouse_Lock(int window);
void Kinc_Internal_Mouse_Unlock(int window);
void Kinc_Internal_Mouse_WindowActivated(int window);
void Kinc_Internal_Mouse_WindowDeactivated(int window);

#ifdef __cplusplus
}
#endif
