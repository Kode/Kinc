#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC extern void (*kinc_mouse_press_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
KINC_FUNC extern void (*kinc_mouse_release_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);
KINC_FUNC extern void (*kinc_mouse_move_callback)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/);
KINC_FUNC extern void (*kinc_mouse_scroll_callback)(int /*window*/, int /*delta*/);
KINC_FUNC extern void (*kinc_mouse_enter_window_callback)(int /*window*/);
KINC_FUNC extern void (*kinc_mouse_leave_window_callback)(int /*window*/);

KINC_FUNC bool kinc_mouse_can_lock(int window);
KINC_FUNC bool kinc_mouse_is_locked(int window);
KINC_FUNC void kinc_mouse_lock(int window);
KINC_FUNC void kinc_mouse_unlock(int window);

KINC_FUNC void kinc_mouse_set_cursor(int cursor);

KINC_FUNC void kinc_mouse_show();
KINC_FUNC void kinc_mouse_hide();
KINC_FUNC void kinc_mouse_set_position(int window, int x, int y);
KINC_FUNC void kinc_mouse_get_position(int window, int *x, int *y);

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
