#pragma once

#include <kinc/global.h>

/*! \file gamepad.h
    \brief Provides gamepad-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Is called with data about changing gamepad-sticks.
/// </summary>
KINC_FUNC extern void (*kinc_gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/);

/// <summary>
/// Is called with data about changing gamepad-buttons.
/// </summary>
KINC_FUNC extern void (*kinc_gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/);

/// <summary>
/// Returns a vendor-name for a gamepad.
/// </summary>
/// <param name="gamepad">The index of the gamepad for which to receive the vendor-name</param>
/// <returns>The vendor-name</returns>
KINC_FUNC const char *kinc_gamepad_vendor(int gamepad);

/// <summary>
/// Returns a name for a gamepad.
/// </summary>
/// <param name="gamepad">The index of the gamepad for which to receive the name</param>
/// <returns>The gamepad's name</returns>
KINC_FUNC const char *kinc_gamepad_product_name(int gamepad);

/// <summary>
/// Checks whether a gamepad is connected.
/// </summary>
/// <param name="gamepad">The index of the gamepad which's connection will be checked</param>
/// <returns>Whether a gamepad is connected for the gamepad-index</returns>
KINC_FUNC bool kinc_gamepad_connected(int gamepad);

/// <summary>
/// Rumbles a gamepad. Careful here because it might just fall of your table.
/// </summary>
/// <param name="gamepad">The index of the gamepad to rumble</param>
/// <param name="left">Rumble-strength for the left motor between 0 and 1</param>
/// <param name="right">Rumble-strength for the right motor between 0 and 1</param>
KINC_FUNC void kinc_gamepad_rumble(int gamepad, float left, float right);

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);

#ifdef __cplusplus
}
#endif
