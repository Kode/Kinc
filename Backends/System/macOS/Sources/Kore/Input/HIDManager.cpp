#include "pch.h"

#include <Kore/Input/Gamepad.h>
#include <Kore/Input/HIDManager.h>

#include <Kore/Log.h>

using namespace Kore;

namespace {
	int maxPadCount = 15;
	int connectedGamepads = 0;
	bool* gamepadList = new bool[maxPadCount];
}

HIDManager::HIDManager() : managerRef(0x0) {
	initHIDManager();
}

HIDManager::~HIDManager() {

	if (managerRef) {
		IOHIDManagerClose(managerRef, kIOHIDOptionsTypeNone);
		IOHIDManagerUnscheduleFromRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		// Unregister
		// IOHIDManagerRegisterInputValueCallback(managerRef, NULL, this);
		// IOHIDManagerRegisterDeviceMatchingCallback(managerRef, NULL, this);
		// IOHIDManagerRegisterDeviceRemovalCallback(managerRef, NULL, this);
	}
}

int HIDManager::initHIDManager() {

	// Initialize the IOHIDManager
	managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(managerRef) == IOHIDManagerGetTypeID()) {

		// Create a matching dictionary for gamepads and joysticks
		CFMutableArrayRef matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (matchingCFArrayRef) {
			// Create a device matching dictionary for joysticks
			CFDictionaryRef matchingCFDictRef = createDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			addMatchingArray(matchingCFArrayRef, matchingCFDictRef);

			// Create a device matching dictionary for game pads
			matchingCFDictRef = createDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			addMatchingArray(matchingCFArrayRef, matchingCFDictRef);
		}
		else {
			log(Error, "%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__);
			return -1;
		}

		// Set the HID device matching array
		IOHIDManagerSetDeviceMatchingMultiple(managerRef, matchingCFArrayRef);
		CFRelease(matchingCFArrayRef);

		// Open manager
		IOHIDManagerOpen(managerRef, kIOHIDOptionsTypeNone);

		// Register routines to be called when (matching) devices are connected or disconnected
		IOHIDManagerRegisterDeviceMatchingCallback(managerRef, deviceConnected, this);
		IOHIDManagerRegisterDeviceRemovalCallback(managerRef, deviceRemoved, this);

		IOHIDManagerScheduleWithRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		// Check how many devices are connected
		CFSetRef deviceSetRef = IOHIDManagerCopyDevices(managerRef);
		if (deviceSetRef) {
			// CFIndex num_devices = CFSetGetCount(deviceSetRef);
			// log(Info, "%d gamepad(s) found.\n",(int)num_devices);
			CFRelease(deviceSetRef);
		}

		return 0;
	}
	return -1;
}

bool HIDManager::addMatchingArray(CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef) {
	if (matchingCFDictRef) {
		// Add it to the matching array
		CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
		CFRelease(matchingCFDictRef); // and release it
		return true;
	}
	return false;
}

// Create matching dictionary
CFMutableDictionaryRef HIDManager::createDeviceMatchingDictionary(u32 inUsagePage, u32 inUsage) {
	// Create a dictionary to add usage page/usages to
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (result) {
		if (inUsagePage) {
			// Add key for device type to refine the matching dictionary.
			CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
			if (pageCFNumberRef) {
				CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
				CFRelease(pageCFNumberRef);

				// note: the usage is only valid if the usage page is also defined
				if (inUsage) {
					CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
					if (usageCFNumberRef) {
						CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
						CFRelease(usageCFNumberRef);
					}
					else {
						log(Error, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
					}
				}
			}
			else {
				log(Error, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
			}
		}
	}
	else {
		log(Error, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
	}
	return result;
}

// This will be called when the HID Manager matches a new (hot plugged) HID device
void HIDManager::deviceConnected(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef) {

	// Get a free index
	int padIndex = -1;
	for (int i = 0; i < maxPadCount; ++i) {
		if (!gamepadList[i]) {
			padIndex = i;
			break;
		}
	}

	if (padIndex == -1) {
		log(Warning, "Too many gamepads connected");
	}

	// HIDGamepad* hidDevice = new HIDGamepad(inIOHIDDeviceRef, padIndex);
	gamepadList[padIndex] = true;
	++connectedGamepads;

	log(Info, "HID device plugged. %i gamepad(s) connected.\n", connectedGamepads);
}

// This will be called when a HID device is removed (unplugged)
void HIDManager::deviceRemoved(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	--connectedGamepads;
	log(Info, "HID device removed. %i gamepad(s) connected.\n", connectedGamepads);
}

void HIDManager::cleanupPad(int index) {
	gamepadList[index] = false;
}
