#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*kinc_gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/);
extern void (*kinc_gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/);
const char *kinc_gamepad_vendor(int gamepad);
const char *kinc_gamepad_product_name(int gamepad);
bool kinc_gamepad_connected(int gamepad);

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);

#ifdef __cplusplus
}
#endif
