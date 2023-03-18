#pragma once

#include <kinc/global.h>

/*! \file gamepad.h
    \brief Provides gamepad-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Sets the gamepad-axis-callback which is called with data about changing gamepad-sticks.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/));

/// <summary>
/// Sets the gamepad-button-callback which is called with data about changing gamepad-buttons.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/));

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
/// Rumbles a gamepad. Careful here because it might just fall off your table.
/// </summary>
/// <param name="gamepad">The index of the gamepad to rumble</param>
/// <param name="left">Rumble-strength for the left motor between 0 and 1</param>
/// <param name="right">Rumble-strength for the right motor between 0 and 1</param>
KINC_FUNC void kinc_gamepad_rumble(int gamepad, float left, float right);

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <memory.h>

static void (*gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/) = NULL;
static void (*gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/) = NULL;

void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/)) {
	gamepad_axis_callback = value;
}

void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/)) {
	gamepad_button_callback = value;
}

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value) {
	if (gamepad_axis_callback != NULL) {
		gamepad_axis_callback(gamepad, axis, value);
	}
}

void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value) {
	if (gamepad_button_callback != NULL) {
		gamepad_button_callback(gamepad, button, value);
	}
}

#endif

#ifdef __cplusplus
}
#endif
