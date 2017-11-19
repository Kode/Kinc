// Modified version of the joystick/gamepad implemenation found in SFML
// https://github.com/SFML/SFML/blob/master/src/SFML/Window/Unix/JoystickImpl.cpp

#include <Kore/Input/Gamepad.h>
#include <Kore/Input/GamepadManager.h>
#include <Kore/Log.h>

#include <cstring>
#include <fcntl.h>
#include <libudev.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <vector>

namespace {
	udev* udevContext = 0;
	udev_monitor* udevMonitor = 0;

	struct GamepadRecord {
		std::string deviceNode;
		std::string systemPath;
		bool plugged;
	};

	typedef std::vector<GamepadRecord> GamepadList;
	GamepadList gamepadList;

	bool isGamepad(udev_device* udevDevice) {
		// If anything goes wrong, we go safe and return true

		// No device to check, assume not a gamepad
		if (!udevDevice) return false;

		const char* devnode = udev_device_get_devnode(udevDevice);

		// We only consider devices with a device node
		if (!devnode) return false;

		// Make sure we only handle /js nodes
		if (!std::strstr(devnode, "/js")) return false;

		// Check if this device is a gamepad
		if (udev_device_get_property_value(udevDevice, "ID_INPUT_JOYSTICK")) return true;

		// Check if this device is something that isn't a gamepad
		// We do this because the absence of any ID_INPUT_ property doesn't
		// necessarily mean that the device isn't a gamepad, whereas the
		// presence of any ID_INPUT_ property that isn't ID_INPUT_JOYSTICK does
		if (udev_device_get_property_value(udevDevice, "ID_INPUT_ACCELEROMETER") ||
            udev_device_get_property_value(udevDevice, "ID_INPUT_KEY") ||
		    udev_device_get_property_value(udevDevice, "ID_INPUT_KEYBOARD") ||
            udev_device_get_property_value(udevDevice, "ID_INPUT_MOUSE") ||
		    udev_device_get_property_value(udevDevice, "ID_INPUT_TABLET") ||
            udev_device_get_property_value(udevDevice, "ID_INPUT_TOUCHPAD") ||
		    udev_device_get_property_value(udevDevice, "ID_INPUT_TOUCHSCREEN"))
			return false;

		// On some platforms (older udev), ID_INPUT_ properties are not present, instead
		// the system makes use of the ID_CLASS property to identify the device class
		const char* idClass = udev_device_get_property_value(udevDevice, "ID_CLASS");

		if (idClass) {
			// Check if the device class matches gamepad
			if (std::strstr(idClass, "joystick")) return true;

			// Check if the device class matches something that isn't a joystick
			// Rationale same as above
			if (std::strstr(idClass, "accelerometer") ||
                std::strstr(idClass, "key") ||
                std::strstr(idClass, "keyboard") ||
                std::strstr(idClass, "mouse") ||
			    std::strstr(idClass, "tablet") ||
                std::strstr(idClass, "touchpad") ||
                std::strstr(idClass, "touchscreen"))
				return false;
		}

		// At this point, assume it is a gamepad
		return true;
	}

	void updatePluggedList(udev_device* udevDevice = NULL) {
		if (udevDevice) {
			const char* action = udev_device_get_action(udevDevice);

			if (action) {
				if (isGamepad(udevDevice)) {
					// Since isGamepad returned true, this has to succeed
					const char* devnode = udev_device_get_devnode(udevDevice);

					GamepadList::iterator record;

					for (record = gamepadList.begin(); record != gamepadList.end(); ++record) {
						if (record->deviceNode == devnode) {
							if (std::strstr(action, "add")) {
								// The system path might have changed so update it
								const char* syspath = udev_device_get_syspath(udevDevice);

								record->plugged = true;
								record->systemPath = syspath ? syspath : "";
								break;
							}
							else if (std::strstr(action, "remove")) {
								record->plugged = false;
								break;
							}
						}
					}

					if (record == gamepadList.end()) {
						if (std::strstr(action, "add")) {
							// If not mapped before and it got added, map it now
							const char* syspath = udev_device_get_syspath(udevDevice);

							GamepadRecord record;
							record.deviceNode = devnode;
							record.systemPath = syspath ? syspath : "";
							record.plugged = true;

							gamepadList.push_back(record);
						}
						else if (std::strstr(action, "remove")) {
							// Not mapped during the initial scan, and removed (shouldn't happen)
							log(Kore::LogLevel::Warning, "Trying to disconnect gamepad that wasn't connected");
						}
					}
				}

				return;
			}
		}

        // Do a full rescan if there was no action just to be sure
		// Reset the plugged status of each mapping since we are doing a full rescan
		for (GamepadList::iterator record = gamepadList.begin(); record != gamepadList.end(); ++record) record->plugged = false;

		udev_enumerate* udevEnumerator = udev_enumerate_new(udevContext);

		if (!udevEnumerator) {
			log(Kore::LogLevel::Warning, "Error while creating udev enumerator");
			return;
		}

		int result = 0;

		result = udev_enumerate_add_match_subsystem(udevEnumerator, "input");

		if (result < 0) {
			log(Kore::LogLevel::Warning, "Error while adding udev enumerator match");
			return;
		}

		result = udev_enumerate_scan_devices(udevEnumerator);

		if (result < 0) {
			log(Kore::LogLevel::Warning, "Error while enumerating udev devices");
			return;
		}

		udev_list_entry* devices = udev_enumerate_get_list_entry(udevEnumerator);
		udev_list_entry* device;

		udev_list_entry_foreach(device, devices) {
			const char* syspath = udev_list_entry_get_name(device);
			udev_device* udevDevice = udev_device_new_from_syspath(udevContext, syspath);

			if (udevDevice && isGamepad(udevDevice)) {
				// Since isGamepad returned true, this has to succeed
				const char* devnode = udev_device_get_devnode(udevDevice);

				GamepadList::iterator record;

				// Check if the device node has been mapped before
				for (record = gamepadList.begin(); record != gamepadList.end(); ++record) {
					if (record->deviceNode == devnode) {
						record->plugged = true;
						break;
					}
				}

				// If not mapped before, map it now
				if (record == gamepadList.end()) {
					GamepadRecord record;
					record.deviceNode = devnode;
					record.systemPath = syspath;
					record.plugged = true;

					gamepadList.push_back(record);
				}
			}

			udev_device_unref(udevDevice);
		}

		udev_enumerate_unref(udevEnumerator);
	}

	bool hasMonitorEvent() {
		// This will not fail since we make sure udevMonitor is valid
		int monitorFd = udev_monitor_get_fd(udevMonitor);

		fd_set descriptorSet;
		FD_ZERO(&descriptorSet);
		FD_SET(monitorFd, &descriptorSet);
		timeval timeout = {0, 0};

		return (select(monitorFd + 1, &descriptorSet, NULL, NULL, &timeout) > 0) && FD_ISSET(monitorFd, &descriptorSet);
	}

	// Get a system attribute from a USB device
	const char* getUsbAttribute(udev_device* udevDevice, const std::string& attributeName) {
		udev_device* udevDeviceParent = udev_device_get_parent_with_subsystem_devtype(udevDevice, "usb", "usb_device");

		if (!udevDeviceParent) return NULL;

		return udev_device_get_sysattr_value(udevDeviceParent, attributeName.c_str());
	}

	// Get the gamepad name
	std::string getGamepadName(unsigned int index) {
		std::string devnode = gamepadList[index].deviceNode;

		// First try using ioctl with JSIOCGNAME
		int fd = ::open(devnode.c_str(), O_RDONLY | O_NONBLOCK);

		if (fd >= 0) {
			// Get the name
			char name[128];
			std::memset(name, 0, sizeof(name));

			int result = ioctl(fd, JSIOCGNAME(sizeof(name)), name);

			::close(fd);

			if (result >= 0) return std::string(name);
		}

		// Fall back to manual USB chain walk via udev
		if (udevContext) {
			udev_device* udevDevice = udev_device_new_from_syspath(udevContext, gamepadList[index].systemPath.c_str());

			if (udevDevice) {
				const char* product = getUsbAttribute(udevDevice, "product");
				udev_device_unref(udevDevice);

				if (product) return std::string(product);
			}
		}

		log(Kore::LogLevel::Warning, "Unable to get name for gamepad %s", devnode);
		return std::string("Unknown Gamepad");
	}
} // namespace

namespace Kore {

	GamepadManager::GamepadManager() {
		initialize();
	}

	GamepadManager::~GamepadManager() {
		terminate();
	}

	void GamepadManager::initialize() {
		// Fill vector with initial gamepads
		gamePads = 12; // From Kore/Input/Gamepad.cpp
		for (unsigned int i = 0; i < gamePads; i++) {
			gamepadsVector.push_back(LinuxGamepad(i));
		}

		// Create udev context
		udevContext = udev_new();

		if (!udevContext) {
			log(Warning, "Failed to create udev context, gamepad support not available");
			return;
		}

		// Create udev monitor
		udevMonitor = udev_monitor_new_from_netlink(udevContext, "udev");

		if (!udevMonitor) {
			log(Warning, "Failed to create udev monitor, gamepad connections and disconnections won't be notified");
		}
		else {
			int error = udev_monitor_filter_add_match_subsystem_devtype(udevMonitor, "input", NULL);

			if (error < 0) {
				log(Warning, "Failed to add udev monitor filter, gamepad connections and disconnections won't be notified: %i", error);

				udev_monitor_unref(udevMonitor);
				udevMonitor = 0;
			}
			else {
				error = udev_monitor_enable_receiving(udevMonitor);

				if (error < 0) {
					log(Warning, "Failed to enable udev monitor, gamepad connections and disconnections won't be notified: %i", error);

					udev_monitor_unref(udevMonitor);
					udevMonitor = 0;
				}
			}
		}

		// Do an initial scan
		updatePluggedList();
	}

	void GamepadManager::update() {
		// Update all gamepads
		for (int i = 0; i < gamepadsVector.size(); ++i) {
			// Update if connectes
			if (gamepadsVector[i].connected) {
				// Try to update gamepad and get connection status
				gamepadsVector[i].connected = gamepadsVector[i].update();

				// Check if it's still connected
				if (!gamepadsVector[i].connected) {
					gamepadsVector[i].close();
					gamepadsVector[i].setInitState(i);
				}
			}
			else {
				// Check if the gamepad was connected since last update
				if (isGamepadConnected(i)) {
					// Open gamepad
					if (gamepadsVector[i].open()) {
						// And update
						gamepadsVector[i].update();
					}
				}
			}
		}
	}

	bool GamepadManager::isGamepadConnected(unsigned int index) {
		// See if we can skip scanning if udev monitor is available
		if (!udevMonitor) {
			// udev monitor is not available, perform a scan every query
			updatePluggedList();
		}
		else if (hasMonitorEvent()) {
			// Check if new gamepads were added/removed since last update
			udev_device* udevDevice = udev_monitor_receive_device(udevMonitor);

			// If we can get the specific device, we check that,
			// otherwise just do a full scan if udevDevice == NULL
			updatePluggedList(udevDevice);

			if (udevDevice) {
				udev_device_unref(udevDevice);
			}
		}

		if (index >= gamepadList.size()) {
			return false;
		}

		// Then check if the gamepad is connected
		return gamepadList[index].plugged;
	}

	void GamepadManager::terminate() {
		// Close file descriptor for open gamepads
		for (unsigned int i = 0; i < gamepadsVector.size(); i++) {
			if (gamepadsVector[i].file_descriptor >= 0) {
				gamepadsVector[i].close();
			}
		}

		// Unreference the udev monitor to destroy it
		if (udevMonitor) {
			udev_monitor_unref(udevMonitor);
			udevMonitor = 0;
		}

		// Unreference the udev context to destroy it
		if (udevContext) {
			udev_unref(udevContext);
			udevContext = 0;
		}
	}

	void LinuxGamepad::setInitState(unsigned int idx) {
		index = idx;
		connected = false;
		file_descriptor = -1;
		name = "No gamepad";
	}

	bool LinuxGamepad::open() {
		if (gamepadList[index].plugged) {
			std::string devnode = gamepadList[index].deviceNode;

			// Open the gamepad's file descriptor (read-only and non-blocking)
			file_descriptor = ::open(devnode.c_str(), O_RDONLY | O_NONBLOCK);
			if (file_descriptor >= 0) {
				// Get info
				name = getGamepadName(index);

				// Set info
				Kore::Gamepad::get(index)->vendor = "Linux gamepad";
				Kore::Gamepad::get(index)->productName = name.c_str();
				return true;
			}
			else {
				std::string errno_str = strerror(errno);
				log(Warning, "Failed to open gamepad %s: %s", devnode, errno_str);
			}
		}
		return false;
	}

	bool LinuxGamepad::update() {
		if (file_descriptor < 0) {
			log(Warning, "Failed to update gamepad, file descriptor error.");
			return false;
		}

		// pop events from the gamepad file
		js_event event;
		int result = read(file_descriptor, &event, sizeof(event));
		while (result > 0) {
			switch (event.type & ~JS_EVENT_INIT) {
			// An axis was moved
			case JS_EVENT_AXIS: {
				if (Kore::Gamepad::get(index)->Axis != nullptr) Kore::Gamepad::get(index)->Axis(event.number, event.value / 32767.f);
				break;
			}

			// A button was pressed
			case JS_EVENT_BUTTON: {
				if (Kore::Gamepad::get(index)->Button != nullptr) Kore::Gamepad::get(index)->Button(event.number, event.value);
				break;
			}
			}

			result = read(file_descriptor, &event, sizeof(event));
		}

		// Check the connection state of the gamepad
		// read() returns -1 and errno != EGAIN if it's no longer connected
		// We need to check the result of read() as well, since errno could
		// have been previously set by some other function call that failed
		// result can be either negative or 0 at this point
		// If result is 0, assume the gamepad is still connected
		// If result is negative, check errno and disconnect if it is not EAGAIN
		return connected = (!result || (errno == EAGAIN));
	}

	void LinuxGamepad::close() {
		::close(file_descriptor);
		file_descriptor = -1;
	}

} // namespace Kore
