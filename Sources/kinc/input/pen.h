#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*kinc_pen_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
extern void (*kinc_pen_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
extern void (*kinc_pen_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure);

#ifdef __cplusplus
}
#endif
