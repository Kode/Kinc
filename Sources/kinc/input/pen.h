#pragma once

#include <kinc/global.h>

/*! \file pen.h
    \brief Provides pen-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Sets the pen-press-callback which is called when the pen is touching the drawing-surface.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_pen_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the pen-move-callback which is called when the pen is moved.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_pen_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the pen-release-callback which is called when the pen is moved away from the drawing-surface.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_pen_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-press-callback which is called when the pen is touching the drawing-surface in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_eraser_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-move-callback which is called when the pen is moved while in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_eraser_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-release-callback which is called when the pen is moved away from the drawing-surface when in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_eraser_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure);

void kinc_internal_eraser_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_release(int window, int x, int y, float pressure);

#ifdef __cplusplus
}
#endif
