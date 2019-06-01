#include "../pch.h"

#include "Gamepad.h"

#include <kinc/input/gamepad.h>

#include <fcntl.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace Kore;

namespace {
	struct HIDGamepad {
		HIDGamepad();
		~HIDGamepad();
		void init(int index);
		void update();

		int idx;
		char gamepad_dev_name[256];
		char name[384];
		int file_descriptor;
		int connected;
		js_event gamepadEvent;

		void open();
		void close();
		void processEvent(js_event e);
	};

	HIDGamepad::HIDGamepad() {}

	void HIDGamepad::init(int index) {
		file_descriptor = -1;
		connected = -1;
		gamepad_dev_name[0] = 0;
		if (index >= 0 && index < 12) {
			idx = index;
			snprintf(gamepad_dev_name, sizeof(gamepad_dev_name), "/dev/input/js%d", idx);
			open();
		}
	}

	HIDGamepad::~HIDGamepad() {
		close();
	}

	void HIDGamepad::open() {
		file_descriptor = ::open(gamepad_dev_name, O_RDONLY | O_NONBLOCK);
		if (file_descriptor < 0) {
			connected = -1;
		}
		else {
			connected = 0;

			char buf[128];
			if (ioctl(file_descriptor, JSIOCGNAME(sizeof(buf)), buf) < 0) strncpy(buf, "Unknown", sizeof(buf));
			snprintf(name, sizeof(name), "%s%s%s%s", buf, " (", gamepad_dev_name, ")");
		}
	}

	void HIDGamepad::close() {
		if (connected == 0) {
			::close(file_descriptor);
			file_descriptor = -1;
			connected = -1;
		}
	}

	void HIDGamepad::update() {
		if (connected == 0) {
			while (read(file_descriptor, &gamepadEvent, sizeof(gamepadEvent)) > 0) {
				processEvent(gamepadEvent);
			}
		}
	}

	void HIDGamepad::processEvent(js_event e) {
		switch (e.type) {
		case JS_EVENT_BUTTON:
			Kinc_Internal_Gamepad_TriggerButton(idx, e.number, e.value);
			break;
		case JS_EVENT_AXIS: {
            float value = e.number % 2 == 0 ? e.value : -e.value;
            Kinc_Internal_Gamepad_TriggerAxis(idx, e.number, value / 32767.0f);
            break;
        }
		default:
			break;
		}
	}

	const int gamepadCount = 12;
	HIDGamepad gamepads[gamepadCount];
}

void Kore::initHIDGamepads() {
	for (int i = 0; i < gamepadCount; ++i) {
		gamepads[i].init(i);
	}
}

void Kore::updateHIDGamepads() {
	for (int i = 0; i < gamepadCount; ++i) {
		gamepads[i].update();
	}
}

const char *Kinc_Gamepad_Vendor(int gamepad) {
    return "Linux gamepad";
}

const char *Kinc_Gamepad_ProductName(int gamepad) {
    return gamepads[gamepad].name;
}
