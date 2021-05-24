#pragma once

#include <kinc/global.h>

/*! \file pen.h
    \brief Provides pen-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Called when the pen is touching the drawing-surface.
/// </summary>
KINC_FUNC extern void (*kinc_pen_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

/// <summary>
/// Called when the pen is moved.
/// </summary>
KINC_FUNC extern void (*kinc_pen_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

/// <summary>
/// Called when the pen is moved away from the drawing-surface.
/// </summary>
KINC_FUNC extern void (*kinc_pen_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

/// <summary>
/// Called when the pen is touching the drawing-surface in eraser-mode.
/// </summary>
KINC_FUNC extern void (*kinc_eraser_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

/// <summary>
/// Called when the pen is moved while in eraser-mode.
/// </summary>
KINC_FUNC extern void (*kinc_eraser_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

/// <summary>
/// Called when the pen is moved away from the drawing-sufrace when in eraser-mode.
/// </summary>
KINC_FUNC extern void (*kinc_eraser_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure);

void kinc_internal_eraser_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_release(int window, int x, int y, float pressure);

#ifdef __cplusplus
}
#endif
