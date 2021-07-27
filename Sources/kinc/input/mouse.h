#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file mouse.h
    \brief Provides mouse-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Is called when a mouse-button is pressed.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_press_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);

/// <summary>
/// Is called when a mouse-button is released.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_release_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/);

/// <summary>
/// Is called when the mouse is moved.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_move_callback)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/);

/// <summary>
/// Is called when the mouse wheel is spinning.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_scroll_callback)(int /*window*/, int /*delta*/);

/// <summary>
/// Is called when the mouse-cursor enters the application-window.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_enter_window_callback)(int /*window*/);

/// <summary>
/// Is called when the mouse-cursor leaves the application-window.
/// </summary>
KINC_FUNC extern void (*kinc_mouse_leave_window_callback)(int /*window*/);

/// <summary>
/// Figures out whether mouse-locking is supported.
/// </summary>
/// <returns>Whether mouse-locking is supported</returns>
KINC_FUNC bool kinc_mouse_can_lock(void);

/// <summary>
/// Figures out whether the mouse is currently locked.
/// </summary>
/// <returns>Whether the mouse is currently locked</returns>
KINC_FUNC bool kinc_mouse_is_locked(void);

/// <summary>
/// Locks the mouse to a window.
/// </summary>
/// <param name="window">The window to lock the mouse to</param>
KINC_FUNC void kinc_mouse_lock(int window);

/// <summary>
/// Unlocks the mouse.
/// </summary>
/// <param name=""></param>
/// <returns></returns>
KINC_FUNC void kinc_mouse_unlock(void);

/// <summary>
/// Change the cursor-image to something that's semi-randomly based on the provided int.
/// </summary>
/// <param name="cursor">Defines what the cursor is changed to - somehow</param>
KINC_FUNC void kinc_mouse_set_cursor(int cursor);

/// <summary>
/// Shows the mouse-cursor.
/// </summary>
KINC_FUNC void kinc_mouse_show(void);

/// <summary>
/// Hides the mouse-cursor.
/// </summary>
KINC_FUNC void kinc_mouse_hide(void);

/// <summary>
/// Manually sets the mouse-cursor-position.
/// </summary>
/// <param name="window">The window to place the cursor in</param>
/// <param name="x">The x-position inside the window to place the cursor at</param>
/// <param name="y">The y-position inside the window to place the cursor at</param>
KINC_FUNC void kinc_mouse_set_position(int window, int x, int y);

/// <summary>
/// Gets the current mouse-position relative to a window.
/// </summary>
/// <param name="window">The window to base the returned position on</param>
/// <param name="x">A pointer where the cursor's x-position is put into</param>
/// <param name="y">A pointer where the cursor's y-position is put into</param>
KINC_FUNC void kinc_mouse_get_position(int window, int *x, int *y);

void kinc_internal_mouse_trigger_press(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_release(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_move(int window, int x, int y);
void kinc_internal_mouse_trigger_scroll(int window, int delta);
void kinc_internal_mouse_trigger_enter_window(int window);
void kinc_internal_mouse_trigger_leave_window(int window);
void kinc_internal_mouse_lock(int window);
void kinc_internal_mouse_unlock(void);
void kinc_internal_mouse_window_activated(int window);
void kinc_internal_mouse_window_deactivated(int window);

#ifdef __cplusplus
}
#endif
