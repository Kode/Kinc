#include "pch.h"

#include "keyboard.h"

#include <memory.h>

void (*Kinc_Keyboard_KeyDownCallback)(int /*key_code*/) = NULL;
void (*Kinc_Keyboard_KeyUpCallback)(int /*key_code*/) = NULL;
void (*Kinc_Keyboard_KeyPressCallback)(unsigned /*character*/) = NULL;

void Kinc_Internal_Keyboard_TriggerKeyDown(int key_code) {
	if (Kinc_Keyboard_KeyDownCallback != NULL) {
		Kinc_Keyboard_KeyDownCallback(key_code);
	}
}

void Kinc_Internal_Keyboard_TriggerKeyUp(int key_code) {
	if (Kinc_Keyboard_KeyUpCallback != NULL) {
		Kinc_Keyboard_KeyUpCallback(key_code);
	}
}

void Kinc_Internal_Keyboard_TriggerKeyPress(unsigned character) {
	if (Kinc_Keyboard_KeyPressCallback != NULL) {
		Kinc_Keyboard_KeyPressCallback(character);
	}
}
