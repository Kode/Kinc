#pragma once

#include <kinc/global.h>

/*! \file surface.h
    \brief Provides touch-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Sets the surface-touch-start-callback which is called when a finger-touch is detected.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_surface_set_touch_start_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));

/// <summary>
/// Sets the surface-move-callback which is called when a finger is moving around.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_surface_set_move_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));

/// <summary>
/// Sets the surface-touch-end-callback which is called when a finger disappears. This is usually not a medical emergency.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_surface_set_touch_end_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));

void kinc_internal_surface_trigger_touch_start(int index, int x, int y);
void kinc_internal_surface_trigger_move(int index, int x, int y);
void kinc_internal_surface_trigger_touch_end(int index, int x, int y);

#ifdef __cplusplus
}
#endif
