#pragma once

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC extern void (*kinc_gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/);
KINC_FUNC extern void (*kinc_gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/);
KINC_FUNC const char *kinc_gamepad_vendor(int gamepad);
KINC_FUNC const char *kinc_gamepad_product_name(int gamepad);
KINC_FUNC bool kinc_gamepad_connected(int gamepad);

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);

#ifdef __cplusplus
}
#endif
