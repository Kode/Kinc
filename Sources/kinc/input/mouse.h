#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void (*kinc_mouse_press_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
extern void (*kinc_mouse_release_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
extern void (*kinc_mouse_move_callback)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/);
extern void (*kinc_mouse_scroll_callback)(int /*window*/, int /*delta*/);
extern void (*kinc_mouse_enter_window_callback)(int /*window*/);
extern void (*kinc_mouse_leave_window_callback)(int /*window*/);

bool kinc_mouse_can_lock(int window);
bool kinc_mouse_is_locked(int window);
void kinc_mouse_lock(int window);
void kinc_mouse_unlock(int window);

void kinc_mouse_set_cursor(int cursor);

void kinc_mouse_show();
void kinc_mouse_hide();
void kinc_mouse_set_position(int window, int x, int y);
void kinc_mouse_get_position(int window, int *x, int *y);

void kinc_internal_mouse_trigger_press(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_release(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_move(int window, int x, int y);
void kinc_internal_mouse_trigger_scroll(int window, int delta);
void kinc_internal_mouse_trigger_enter_window(int window);
void kinc_internal_mouse_trigger_leave_window(int window);
void kinc_internal_mouse_lock(int window);
void kinc_internal_mouse_unlock(int window);
void kinc_internal_mouse_window_activated(int window);
void kinc_internal_mouse_window_deactivated(int window);

#ifdef __cplusplus
}
#endif
