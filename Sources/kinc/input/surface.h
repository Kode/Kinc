#pragma once

#include <kinc/global.h>

/*! \file surface.h
    \brief Provides touch-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Called when a finger-touch is detected.
/// </summary>
KINC_FUNC extern void (*kinc_surface_touch_start_callback)(int /*index*/, int /*x*/, int /*y*/);

/// <summary>
/// Called when a finger is moving around.
/// </summary>
KINC_FUNC extern void (*kinc_surface_move_callback)(int /*index*/, int /*x*/, int /*y*/);

/// <summary>
/// Called when a finger disappears. This is usually not a medical emergency.
/// </summary>
KINC_FUNC extern void (*kinc_surface_touch_end_callback)(int /*index*/, int /*x*/, int /*y*/);

void kinc_internal_surface_trigger_touch_start(int index, int x, int y);
void kinc_internal_surface_trigger_move(int index, int x, int y);
void kinc_internal_surface_trigger_touch_end(int index, int x, int y);

#ifdef __cplusplus
}
#endif
