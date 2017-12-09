#include "HIDGamepad.h"
#include "../pch.h"
#include <Kore/Input/Gamepad.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

using namespace Kore;

HIDGamepad::HIDGamepad(int index) {
	file_descriptor = -1;
	connected = -1;
	gamepad_dev_name[0] = 0;
	if (index >= 0 && index < 12) {
		idx = index;
		sprintf(gamepad_dev_name, "/dev/input/js%d", idx);
		Open();
	}
}

HIDGamepad::~HIDGamepad() {
	Close();
}

int HIDGamepad::Open() {
	file_descriptor = open(gamepad_dev_name, O_RDONLY | O_NONBLOCK);
	if (file_descriptor < 0) {
		connected = -1;
	}
	else {
		connected = 0;
		Gamepad::get(idx)->vendor = "Linux gamepad";
		Gamepad::get(idx)->productName = gamepad_dev_name;
	}
}

int HIDGamepad::Close() {
	if (connected == 0) {
		close(file_descriptor);
		file_descriptor = -1;
		connected = -1;
	}
}

void HIDGamepad::Update() {
	if (connected == 0) {
		while (read(file_descriptor, &gamepadEvent, sizeof(gamepadEvent)) > 0) {
			ProcessEvent(gamepadEvent);
		}
	}
}

void HIDGamepad::ProcessEvent(struct js_event e) {
	switch (e.type) {
	case JS_EVENT_BUTTON:
		if (Gamepad::get(idx)->Button != nullptr) {
			Gamepad::get(idx)->Button(e.number, e.value);
		}
		break;
	case JS_EVENT_AXIS:
		if (Gamepad::get(idx)->Axis != nullptr) {
			Gamepad::get(idx)->Axis(e.number, e.value / 32767.f);
		}
		break;
	default:
		break;
	}
}
