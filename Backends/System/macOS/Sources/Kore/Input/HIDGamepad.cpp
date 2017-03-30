#include "pch.h"

#include "HIDGamepad.h"
#include "HIDManager.h"

#include <Kore/Input/Gamepad.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	int axisCount = 6;
	int buttonCount = 15;
	IOHIDElementCookie* axis = new IOHIDElementCookie[axisCount];
	IOHIDElementCookie* buttons = new IOHIDElementCookie[buttonCount];

	char* toString(CFStringRef string) {
		if (string == NULL) {
			return NULL;
		}

		CFIndex length = CFStringGetLength(string);
		CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
		char* buffer = (char*)malloc(maxSize);
		if (CFStringGetCString(string, buffer, maxSize, kCFStringEncodingUTF8)) {
			return buffer;
		}
		return NULL;
	}

	bool debugButtonInput = false;
	void logButton(int buttonIndex, bool pressed) {
		switch (buttonIndex) {
		case 0:
			log(Info, "A Pressed %i", pressed);
			break;

		case 1:
			log(Info, "B Pressed %i", pressed);
			break;

		case 2:
			log(Info, "X Pressed %i", pressed);
			break;

		case 3:
			log(Info, "Y Pressed %i", pressed);
			break;

		case 4:
			log(Info, "Lb Pressed %i", pressed);
			break;

		case 5:
			log(Info, "Rb Pressed %i", pressed);
			break;

		case 6:
			log(Info, "Left Stick Pressed %i", pressed);
			break;

		case 7:
			log(Info, "Right Stick Pressed %i", pressed);
			break;

		case 8:
			log(Info, "Start Pressed %i", pressed);
			break;

		case 9:
			log(Info, "Back Pressed %i", pressed);
			break;

		case 10:
			log(Info, "Home Pressed %i", pressed);
			break;

		case 11:
			log(Info, "Up Pressed %i", pressed);
			break;

		case 12:
			log(Info, "Down Pressed %i", pressed);
			break;

		case 13:
			log(Info, "Left Pressed %i", pressed);
			break;

		case 14:
			log(Info, "Right Pressed %i", pressed);
			break;

		default:
			break;
		}
	}

	bool debugAxisInput = false;
	void logAxis(int axisIndex) {
		switch (axisIndex) {
		case 0:
			log(Info, "Left stick X");
			break;

		case 1:
			log(Info, "Left stick Y");
			break;

		case 2:
			log(Info, "Right stick X");
			break;

		case 3:
			log(Info, "Right stick Y");
			break;

		case 4:
			log(Info, "Left trigger");
			break;

		case 5:
			log(Info, "Right trigger");
			break;

		default:
			break;
		}
	}
}

HIDGamepad::HIDGamepad(IOHIDDeviceRef deviceRef, int padIndex) : deviceRef(deviceRef), padIndex(padIndex) {
	initHIDDevice();

	Gamepad* gamepad = Gamepad::get(padIndex);
	gamepad->vendor = getManufacturerKey();
	gamepad->productName = getProductKey();
	log(Info, "Add gamepad: Vendor: %s, Name: %s", gamepad->vendor, gamepad->productName);
}

HIDGamepad::~HIDGamepad() {
	if (deviceRef) {
		IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeSeizeDevice);
		IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDDeviceRegisterRemovalCallback(deviceRef, NULL, this);
	}

	if (inIOHIDQueueRef) {
		IOHIDQueueStop(inIOHIDQueueRef);
		IOHIDQueueUnscheduleFromRunLoop(inIOHIDQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	delete[] axis;
	delete[] buttons;
	axis = nullptr;
	buttons = nullptr;
}

void HIDGamepad::initHIDDevice() {
	if (deviceRef) {
		// Get all elements for a specific device
		CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(deviceRef, NULL, kIOHIDOptionsTypeNone);

		// Open device
		IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeSeizeDevice);

		// Register routines
		IOHIDDeviceRegisterInputValueCallback(deviceRef, inputValueCallback, this);
		IOHIDDeviceRegisterRemovalCallback(deviceRef, deviceRemovalCallback, this);

		IOHIDDeviceScheduleWithRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		// Create a queue to access the values
		inIOHIDQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, deviceRef, 32, kIOHIDOptionsTypeNone);
		if (CFGetTypeID(inIOHIDQueueRef) == IOHIDQueueGetTypeID()) {
			// this is a valid HID queue reference!
			initElementsFromArray(elementCFArrayRef);
			IOHIDQueueStart(inIOHIDQueueRef);
			IOHIDQueueRegisterValueAvailableCallback(inIOHIDQueueRef, valueAvailableCallback, this);
			IOHIDQueueScheduleWithRunLoop(inIOHIDQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		}
	}
}

void HIDGamepad::initElementsFromArray(CFArrayRef elements) {
	for (CFIndex i = 0, count = CFArrayGetCount(elements); i < count; ++i) {
		IOHIDElementRef elementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		IOHIDElementType elemType = IOHIDElementGetType(elementRef);

		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		uint32_t usagePage = IOHIDElementGetUsagePage(elementRef);
		uint32_t usage = IOHIDElementGetUsage(elementRef);

		// Match up items
		switch (usagePage) {
		case kHIDPage_GenericDesktop:
			switch (usage) {
			case kHIDUsage_GD_X: // Left stick X
				// log(Info, "Left stick X axis[0] = %i", cookie);
				axis[0] = cookie;
				break;
			case kHIDUsage_GD_Y: // Left stick Y
				// log(Info, "Left stick Y axis[1] = %i", cookie);
				axis[1] = cookie;
				break;
			case kHIDUsage_GD_Z: // Left trigger
				// log(Info, "Left trigger axis[4] = %i", cookie);
				axis[4] = cookie;
				break;
			case kHIDUsage_GD_Rx: // Right stick X
				// log(Info, "Right stick X axis[2] = %i", cookie);
				axis[2] = cookie;
				break;
			case kHIDUsage_GD_Ry: // Right stick Y
				// log(Info, "Right stick Y axis[3] = %i", cookie);
				axis[3] = cookie;
				break;
			case kHIDUsage_GD_Rz: // Right trigger
				// log(Info, "Right trigger axis[5] = %i", cookie);
				axis[5] = cookie;
				break;
			case kHIDUsage_GD_Hatswitch:
				break;
			default:
				break;
			}
			break;
		case kHIDPage_Button:
			if ((usage >= 1) && (usage <= 15)) {
				// Button 1-11
				buttons[usage - 1] = cookie;
				// log(Info, "Button %i = %i", usage-1, cookie);
			}
			break;
		default:
			break;
		}

		if (elemType == kIOHIDElementTypeInput_Misc || elemType == kIOHIDElementTypeInput_Button || elemType == kIOHIDElementTypeInput_Axis) {
			if (!IOHIDQueueContainsElement(inIOHIDQueueRef, elementRef)) IOHIDQueueAddElement(inIOHIDQueueRef, elementRef);
		}
	}
}

// Get a HID device's vendor ID (long)
int HIDGamepad::getVendorID() {
	int vendorID = 0;
	CFNumberRef cfVendorID = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDVendorIDKey));
	CFNumberGetValue(cfVendorID, kCFNumberIntType, &vendorID);
	return vendorID;
}

// Get a HID device's product ID (long)
int HIDGamepad::getProductID() {
	int productID = 0;
	CFNumberRef cfVendorID = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDProductIDKey));
	CFNumberGetValue(cfVendorID, kCFNumberIntType, &productID);
	return productID;
}

// Get a HID device's manifacture key (string)
char* HIDGamepad::getManufacturerKey() {
	CFStringRef manufacturerKey = (CFStringRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDManufacturerKey));
	return toString(manufacturerKey);
}

// Get a HID device's product key (string)
char* HIDGamepad::getProductKey() {
	CFStringRef productName = (CFStringRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDProductKey));
	return toString(productName);
}

void HIDGamepad::inputValueCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef inIOHIDValueRef) {}

void HIDGamepad::deviceRemovalCallback(void* inContext, IOReturn inResult, void* inSender) {
	HIDGamepad* pad = (HIDGamepad*)inContext;
	HIDManager::cleanupPad(pad->padIndex);
}

void HIDGamepad::valueAvailableCallback(void* inContext, IOReturn inResult, void* inSender) {
	HIDGamepad* pad = (HIDGamepad*)inContext;
	do {
		IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef)inSender, 0.);
		if (!valueRef) break;
		// process the HID value reference
		IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);
		// IOHIDElementType elemType = IOHIDElementGetType(elementRef);

		// log(Info, "Type %d %d\n", elemType, elementRef);
		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);
		// uint32_t page = IOHIDElementGetUsagePage(elementRef);
		// uint32_t usage = IOHIDElementGetUsage(elementRef);
		// log(Info, "page %i, usage %i cookie %i", page, usage, cookie);

		// Check button
		bool found = false;
		for (int i = 0; i < buttonCount && !found; ++i) {
			if (cookie == buttons[i]) {
				pad->buttonChanged(elementRef, valueRef, i);
				found = true;
			}
		}

		// Check axis
		for (int i = 0; i < axisCount && !found; ++i) {
			if (cookie == axis[i]) {
				pad->axisChanged(elementRef, valueRef, i);
				found = true;
			}
		}

		CFRelease(valueRef);
	} while (1);
}

void HIDGamepad::buttonChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);

	// log(Info, "%f %f %f %f", rawValue, min, max, normalize);

	Gamepad::get(padIndex)->_button(buttonIndex, normalize);

	if (debugButtonInput) logButton(buttonIndex, (normalize != 0));
}

void HIDGamepad::axisChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize axis value to the range [-1.0, 1.0] (e.g. -1 - left, 0 - release, 1 - right)
	double min = IOHIDElementGetPhysicalMin(elementRef);
	double max = IOHIDElementGetPhysicalMax(elementRef);
	double normalize = normalize = (((rawValue - min) / (max - min)) * 2) - 1;

	// Invert Y axis
	if (axisIndex % 2 == 1) normalize = -normalize;

	// log(Info, "%f %f %f %f", rawValue, min, max, normalize);

	Gamepad::get(padIndex)->_axis(axisIndex, normalize);

	if (debugAxisInput) logAxis(axisIndex);
}
