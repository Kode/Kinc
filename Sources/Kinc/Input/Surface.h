#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Kinc_Surface_TouchStartCallback)(int /*index*/, int /*x*/, int /*y*/);
extern void (*Kinc_Surface_MoveCallback)(int /*index*/, int /*x*/, int /*y*/);
extern void (*Kinc_Surface_TouchEndCallback)(int /*index*/, int /*x*/, int /*y*/);

void Kinc_Internal_Surface_TriggerTouchStart(int index, int x, int y);
void Kinc_Internal_Surface_TriggerMove(int index, int x, int y);
void Kinc_Internal_Surface_TriggerTouchEnd(int index, int x, int y);

#ifdef __cplusplus
}
#endif
