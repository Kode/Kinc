#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Kinc_Pen_PressCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
extern void (*Kinc_Pen_MoveCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
extern void (*Kinc_Pen_ReleaseCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

void Kinc_Internal_Pen_TriggerMove(int window, int x, int y, float pressure);
void Kinc_Internal_Pen_TriggerPress(int window, int x, int y, float pressure);
void Kinc_Internal_Pen_TriggerRelease(int window, int x, int y, float pressure);

#ifdef __cplusplus
}
#endif
