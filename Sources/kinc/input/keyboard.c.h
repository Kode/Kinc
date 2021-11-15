#include "keyboard.h"

#include <memory.h>

static void (*keyboard_key_down_callback)(int /*key_code*/) = NULL;
static void (*keyboard_key_up_callback)(int /*key_code*/) = NULL;
static void (*keyboard_key_press_callback)(unsigned /*character*/) = NULL;

void kinc_set_keyboard_key_down_callback(void (*value)(int /*key_code*/)){
	keyboard_key_down_callback = value;
}

void kinc_set_keyboard_key_up_callback(void (*value)(int /*key_code*/)){
	keyboard_key_up_callback = value;
}

void kinc_set_keyboard_key_press_callback(void (*value)(unsigned /*character*/)){
	keyboard_key_press_callback = value;
}

void kinc_internal_keyboard_trigger_key_down(int key_code) {
	if (keyboard_key_down_callback != NULL) {
		keyboard_key_down_callback(key_code);
	}
}

void kinc_internal_keyboard_trigger_key_up(int key_code) {
	if (keyboard_key_up_callback != NULL) {
		keyboard_key_up_callback(key_code);
	}
}

void kinc_internal_keyboard_trigger_key_press(unsigned character) {
	if (keyboard_key_press_callback != NULL) {
		keyboard_key_press_callback(character);
	}
}
