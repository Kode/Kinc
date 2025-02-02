#include <kinc/input/gamepad.h>

void kinc_gamepad_set_count(int count) {}

const char *kinc_gamepad_vendor(int gamepad) {
	return "None";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "Gamepad";
}

bool kinc_gamepad_connected(int gamepad) {
	return false;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {}
