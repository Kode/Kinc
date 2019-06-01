#include "pch.h"

#include "HIDGamepad.h"
#include "HIDManager.h"

#include <kinc/input/gamepad.h>

#include <Kore/Log.h>
#include <Kore/Error.h>
#include <Kore/Math/Core.h>

using namespace Kore;

namespace {
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

// Helper function to copy a CFStringRef to a cstring buffer.
// CFStringRef is converted to UTF8 and as many characters as possible are
// placed into the buffer followed by a null terminator.
// The buffer is set to an empty string if the conversion fails.
static void cstringFromCFStringRef(CFStringRef string, char *cstr, size_t clen) {
	cstr[0] = '\0';
	if (string != NULL) {
		char temp[256];
		if (CFStringGetCString(string, temp, 256, kCFStringEncodingUTF8)) {
			temp[Kore::min(255, (int)(clen-1))] = '\0';
			strncpy(cstr, temp, clen);
		}
	}
}

HIDGamepad::HIDGamepad() {
	reset();
}

HIDGamepad::~HIDGamepad() {
	unbind();
}

void HIDGamepad::bind(IOHIDDeviceRef inDeviceRef, int inPadIndex) {
	Kore::affirm(inDeviceRef != nullptr);
	Kore::affirm(inPadIndex >= 0);
	Kore::affirm(hidDeviceRef == nullptr);
	Kore::affirm(hidQueueRef  == nullptr);
	Kore::affirm(padIndex     == -1);

	// Set device and device index
	hidDeviceRef = inDeviceRef;
	padIndex  	 = inPadIndex;

	// Initialise HID Device
	// ...open device
	IOHIDDeviceOpen(hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);

	// ..register callbacks
	IOHIDDeviceRegisterInputValueCallback(hidDeviceRef, inputValueCallback, this);
	IOHIDDeviceScheduleWithRunLoop(hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	// ...create a queue to access element values
	hidQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, hidDeviceRef, 32, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(hidQueueRef) == IOHIDQueueGetTypeID()) {
		IOHIDQueueStart(hidQueueRef);
		IOHIDQueueRegisterValueAvailableCallback(hidQueueRef, valueAvailableCallback, this);
		IOHIDQueueScheduleWithRunLoop(hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	// ...get all elements (buttons, axes)
	CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(hidDeviceRef, NULL, kIOHIDOptionsTypeNone);
	initDeviceElements(elementCFArrayRef);

	// ...get device manufacturer and product details
	{
		CFNumberRef vendorIdRef = (CFNumberRef) IOHIDDeviceGetProperty(hidDeviceRef, CFSTR(kIOHIDVendorIDKey));
		CFNumberGetValue(vendorIdRef, kCFNumberIntType, &hidDeviceVendorID);

		CFNumberRef productIdRef = (CFNumberRef) IOHIDDeviceGetProperty(hidDeviceRef, CFSTR(kIOHIDProductIDKey));
		CFNumberGetValue(productIdRef, kCFNumberIntType, &hidDeviceProductID);

		CFStringRef vendorRef = (CFStringRef) IOHIDDeviceGetProperty(hidDeviceRef, CFSTR(kIOHIDManufacturerKey));
		cstringFromCFStringRef(vendorRef, hidDeviceVendor, sizeof(hidDeviceVendor));

		CFStringRef productRef = (CFStringRef) IOHIDDeviceGetProperty(hidDeviceRef, CFSTR(kIOHIDProductKey));
		cstringFromCFStringRef(productRef, hidDeviceProduct, sizeof(hidDeviceProduct));
	}

	// Initialise Kore::Gamepad for this HID Device
	//**
	/*Gamepad *gamepad = Gamepad::get(padIndex);
	gamepad->vendor 	 = hidDeviceVendor;
	gamepad->productName = hidDeviceProduct;*/

	Kore::log(Info, "HIDGamepad.bind: <%p> idx:%d [0x%x:0x%x] [%s] [%s]", inDeviceRef, padIndex, hidDeviceVendorID, hidDeviceProductID, hidDeviceVendor, hidDeviceProduct);
}

void HIDGamepad::initDeviceElements(CFArrayRef elements) {
	Kore::affirm(elements != nullptr);

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
			if (!IOHIDQueueContainsElement(hidQueueRef, elementRef)) IOHIDQueueAddElement(hidQueueRef, elementRef);
		}
	}
}

void HIDGamepad::unbind() {
	Kore::log(Info, "HIDGamepad.unbind: idx:%d [0x%x:0x%x] [%s] [%s]", padIndex, hidDeviceVendorID, hidDeviceProductID, hidDeviceVendor, hidDeviceProduct);

	if (hidQueueRef) {
		IOHIDQueueStop(hidQueueRef);
		IOHIDQueueUnscheduleFromRunLoop(hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	if (hidDeviceRef) {
		IOHIDDeviceUnscheduleFromRunLoop(hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDDeviceClose(hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);
	}

	if (padIndex >= 0) {
		//**
		/*Gamepad *gamepad = Gamepad::get(padIndex);
		gamepad->vendor 	 = nullptr;
		gamepad->productName = nullptr;*/
	}

	reset();
}

void HIDGamepad::reset() {
	padIndex			= -1;
	hidDeviceRef		= NULL;
	hidQueueRef 		= NULL;
	hidDeviceVendor[0]  = '\0';
	hidDeviceProduct[0]	= '\0';
	hidDeviceVendorID 	= 0;
	hidDeviceProductID 	= 0;

	memset(axis   , 0, sizeof(axis));
	memset(buttons, 0, sizeof(buttons));
}

void HIDGamepad::buttonChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);

	// log(Info, "%f %f %f %f", rawValue, min, max, normalize);

	Kinc_Internal_Gamepad_TriggerButton(padIndex, buttonIndex, normalize);

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

	Kinc_Internal_Gamepad_TriggerAxis(padIndex, axisIndex, normalize);

	if (debugAxisInput) logAxis(axisIndex);
}

void HIDGamepad::inputValueCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef inIOHIDValueRef) {}

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
		for (int i = 0, c = sizeof(buttons); i < c; ++i) {
			if (cookie == pad->buttons[i]) {
				pad->buttonChanged(elementRef, valueRef, i);
				break;
			}
		}

		// Check axes
		for (int i = 0, c = sizeof(axis); i < c; ++i) {
			if (cookie == pad->axis[i]) {
				pad->axisChanged(elementRef, valueRef, i);
				break;
			}
		}

		CFRelease(valueRef);
	} while (1);
}

const char *Kinc_Gamepad_Vendor(int gamepad) {
	return "unknown";
}

const char *Kinc_Gamepad_ProductName(int gamepad) {
	return "unknown";
}
