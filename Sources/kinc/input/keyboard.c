#include "pch.h"

#include "keyboard.h"

#include <memory.h>

void (*kinc_keyboard_key_down_callback)(int /*key_code*/) = NULL;
void (*kinc_keyboard_key_up_callback)(int /*key_code*/) = NULL;
void (*kinc_keyboard_key_press_callback)(unsigned /*character*/) = NULL;

void kinc_internal_keyboard_trigger_key_down(int key_code) {
	if (kinc_keyboard_key_down_callback != NULL) {
		kinc_keyboard_key_down_callback(key_code);
	}
}

void kinc_internal_keyboard_trigger_key_up(int key_code) {
	if (kinc_keyboard_key_up_callback != NULL) {
		kinc_keyboard_key_up_callback(key_code);
	}
}

void kinc_internal_keyboard_trigger_key_press(unsigned character) {
	if (kinc_keyboard_key_press_callback != NULL) {
		kinc_keyboard_key_press_callback(character);
	}
}
