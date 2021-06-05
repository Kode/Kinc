#include "gamepad.h"

#include <kinc/input/gamepad.h>

#include <fcntl.h>
#include <libudev.h>
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
		bool connected;
		js_event gamepadEvent;

		void open();
		void close();
		void processEvent(js_event e);
	};

	HIDGamepad::HIDGamepad() {}

	void HIDGamepad::init(int index) {
		file_descriptor = -1;
		connected = false;
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
			connected = false;
		}
		else {
			connected = true;

			char buf[128];
			if (ioctl(file_descriptor, JSIOCGNAME(sizeof(buf)), buf) < 0) strncpy(buf, "Unknown", sizeof(buf));
			snprintf(name, sizeof(name), "%s%s%s%s", buf, " (", gamepad_dev_name, ")");
		}
	}

	void HIDGamepad::close() {
		if (connected) {
			::close(file_descriptor);
			file_descriptor = -1;
			connected = false;
		}
	}

	void HIDGamepad::update() {
		if (connected) {
			while (read(file_descriptor, &gamepadEvent, sizeof(gamepadEvent)) > 0) {
				processEvent(gamepadEvent);
			}
		}
	}

	void HIDGamepad::processEvent(js_event e) {
		switch (e.type) {
		case JS_EVENT_BUTTON:
			kinc_internal_gamepad_trigger_button(idx, e.number, e.value);
			break;
		case JS_EVENT_AXIS: {
            float value = e.number % 2 == 0 ? e.value : -e.value;
            kinc_internal_gamepad_trigger_axis(idx, e.number, value / 32767.0f);
            break;
        }
		default:
			break;
		}
	}


	struct HIDGamepadUdevHelper {

	private:

		struct udev* udevPtr;
		struct udev_monitor* udevMonitorPtr;
		int udevMonitorFD;

		void openOrCloseGamepad(struct udev_device* dev);
		void processDevice(struct udev_device* dev);

	public:

		void init();
		void update();
		void close();

	};

	void HIDGamepadUdevHelper::processDevice(struct udev_device* dev) {

	    if (dev) {

		    if (udev_device_get_devnode(dev))
		        HIDGamepadUdevHelper::openOrCloseGamepad(dev);

		    udev_device_unref(dev);

		}

	}

	void HIDGamepadUdevHelper::init() {

	    struct udev* udevPtrNew = udev_new();

	    //enumerate
	    struct udev_enumerate* enumerate = udev_enumerate_new(udevPtrNew);

	    udev_enumerate_add_match_subsystem(enumerate, "input");
	    udev_enumerate_scan_devices(enumerate);

	    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
	    struct udev_list_entry* entry;

	    udev_list_entry_foreach(entry, devices) {
	        const char* path = udev_list_entry_get_name(entry);
	        struct udev_device* dev = udev_device_new_from_syspath(udevPtrNew, path);
	        processDevice(dev);
	    }

	    udev_enumerate_unref(enumerate);

	    //setup mon
	    udevMonitorPtr = udev_monitor_new_from_netlink(udevPtrNew, "udev");

	    udev_monitor_filter_add_match_subsystem_devtype(udevMonitorPtr, "input", NULL);
	    udev_monitor_enable_receiving(udevMonitorPtr);

	    udevMonitorFD = udev_monitor_get_fd(udevMonitorPtr);

		udevPtr = udevPtrNew;

	}

	void HIDGamepadUdevHelper::update() {

	    fd_set fds;
	    FD_ZERO(&fds);
	    FD_SET(udevMonitorFD, &fds);

	    if (FD_ISSET(udevMonitorFD, &fds)) {
	        struct udev_device* dev = udev_monitor_receive_device(udevMonitorPtr);
	        processDevice(dev);
	    }

	}

	void HIDGamepadUdevHelper::close() {

	    udev_unref(udevPtr);

	}

	HIDGamepadUdevHelper udev_helper;

	const int gamepadCount = 12;
	HIDGamepad gamepads[gamepadCount];

	void HIDGamepadUdevHelper::openOrCloseGamepad(struct udev_device* dev) {

		const char* action = udev_device_get_action(dev);
		if (!action) action = "add";

		const char* joystickDevnodeName = strstr(udev_device_get_devnode(dev), "js");

		if (joystickDevnodeName) {


		    int joystickDevnodeIndex;
		    sscanf(joystickDevnodeName, "js%d", &joystickDevnodeIndex);

		    if (!strcmp(action, "add"))
	            gamepads[joystickDevnodeIndex].open();

			if (!strcmp(action, "remove"))
			    gamepads[joystickDevnodeIndex].close();

	    }

	}

}



void Kore::initHIDGamepads() {
	for (int i = 0; i < gamepadCount; ++i) {
		gamepads[i].init(i);
	}
	udev_helper.init();
}

void Kore::updateHIDGamepads() {
	udev_helper.update();
	for (int i = 0; i < gamepadCount; ++i) {
		gamepads[i].update();
	}
}

void Kore::closeHIDGamepads() {
	udev_helper.close();
}

const char *kinc_gamepad_vendor(int gamepad) {
    return "Linux gamepad";
}

const char *kinc_gamepad_product_name(int gamepad) {
    return gamepads[gamepad].name;
}

bool kinc_gamepad_connected(int gamepad) {
	return gamepads[gamepad].connected;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {
	
}