#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Kinc_Gamepad_AxisCallback)(int /*gamepad*/, int /*axis*/, float /*value*/);
extern void (*Kinc_Gamepad_ButtonCallback)(int /*gamepad*/, int /*button*/, float /*value*/);
const char *Kinc_Gamepad_Vendor(int gamepad);
const char *Kinc_Gamepad_ProductName(int gamepad);

void Kinc_Internal_Gamepad_TriggerAxis(int gamepad, int axis, float value);
void Kinc_Internal_Gamepad_TriggerButton(int gamepad, int button, float value);

#ifdef __cplusplus
}
#endif
