#include "HIDGamepad.h"
#include "HIDManager.h"

#include <kinc/input/gamepad.h>
#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

namespace {
	bool debugButtonInput = false;
	void logButton(int buttonIndex, bool pressed) {
		switch (buttonIndex) {
		case 0:
			kinc_log(KINC_LOG_LEVEL_INFO, "A Pressed %i", pressed);
			break;

		case 1:
			kinc_log(KINC_LOG_LEVEL_INFO, "B Pressed %i", pressed);
			break;

		case 2:
			kinc_log(KINC_LOG_LEVEL_INFO, "X Pressed %i", pressed);
			break;

		case 3:
			kinc_log(KINC_LOG_LEVEL_INFO, "Y Pressed %i", pressed);
			break;

		case 4:
			kinc_log(KINC_LOG_LEVEL_INFO, "Lb Pressed %i", pressed);
			break;

		case 5:
			kinc_log(KINC_LOG_LEVEL_INFO, "Rb Pressed %i", pressed);
			break;

		case 6:
			kinc_log(KINC_LOG_LEVEL_INFO, "Left Stick Pressed %i", pressed);
			break;

		case 7:
			kinc_log(KINC_LOG_LEVEL_INFO, "Right Stick Pressed %i", pressed);
			break;

		case 8:
			kinc_log(KINC_LOG_LEVEL_INFO, "Start Pressed %i", pressed);
			break;

		case 9:
			kinc_log(KINC_LOG_LEVEL_INFO, "Back Pressed %i", pressed);
			break;

		case 10:
			kinc_log(KINC_LOG_LEVEL_INFO, "Home Pressed %i", pressed);
			break;

		case 11:
			kinc_log(KINC_LOG_LEVEL_INFO, "Up Pressed %i", pressed);
			break;

		case 12:
			kinc_log(KINC_LOG_LEVEL_INFO, "Down Pressed %i", pressed);
			break;

		case 13:
			kinc_log(KINC_LOG_LEVEL_INFO, "Left Pressed %i", pressed);
			break;

		case 14:
			kinc_log(KINC_LOG_LEVEL_INFO, "Right Pressed %i", pressed);
			break;

		default:
			break;
		}
	}

	bool debugAxisInput = false;
	void logAxis(int axisIndex) {
		switch (axisIndex) {
		case 0:
			kinc_log(KINC_LOG_LEVEL_INFO, "Left stick X");
			break;

		case 1:
			kinc_log(KINC_LOG_LEVEL_INFO, "Left stick Y");
			break;

		case 2:
			kinc_log(KINC_LOG_LEVEL_INFO, "Right stick X");
			break;

		case 3:
			kinc_log(KINC_LOG_LEVEL_INFO, "Right stick Y");
			break;

		case 4:
			kinc_log(KINC_LOG_LEVEL_INFO, "Left trigger");
			break;

		case 5:
			kinc_log(KINC_LOG_LEVEL_INFO, "Right trigger");
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
			temp[kinc_mini(255, (int)(clen-1))] = '\0';
			strncpy(cstr, temp, clen);
		}
	}
}

Kore::HIDGamepad::HIDGamepad() {
	reset();
}

Kore::HIDGamepad::~HIDGamepad() {
	unbind();
}

void Kore::HIDGamepad::bind(IOHIDDeviceRef inDeviceRef, int inPadIndex) {
	kinc_affirm(inDeviceRef != nullptr);
	kinc_affirm(inPadIndex >= 0);
	kinc_affirm(hidDeviceRef == nullptr);
	kinc_affirm(hidQueueRef  == nullptr);
	kinc_affirm(padIndex     == -1);

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

	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.bind: <%p> idx:%d [0x%x:0x%x] [%s] [%s]", inDeviceRef, padIndex, hidDeviceVendorID, hidDeviceProductID, hidDeviceVendor, hidDeviceProduct);
}

void Kore::HIDGamepad::initDeviceElements(CFArrayRef elements) {
	kinc_affirm(elements != nullptr);

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

void Kore::HIDGamepad::unbind() {
	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.unbind: idx:%d [0x%x:0x%x] [%s] [%s]", padIndex, hidDeviceVendorID, hidDeviceProductID, hidDeviceVendor, hidDeviceProduct);

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

void Kore::HIDGamepad::reset() {
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

void Kore::HIDGamepad::buttonChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);

	// log(Info, "%f %f %f %f", rawValue, min, max, normalize);

	kinc_internal_gamepad_trigger_button(padIndex, buttonIndex, normalize);

	if (debugButtonInput) logButton(buttonIndex, (normalize != 0));
}

void Kore::HIDGamepad::axisChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize axis value to the range [-1.0, 1.0] (e.g. -1 - left, 0 - release, 1 - right)
	double min = IOHIDElementGetPhysicalMin(elementRef);
	double max = IOHIDElementGetPhysicalMax(elementRef);
	double normalize = normalize = (((rawValue - min) / (max - min)) * 2) - 1;

	// Invert Y axis
	if (axisIndex % 2 == 1) normalize = -normalize;

	// log(Info, "%f %f %f %f", rawValue, min, max, normalize);

	kinc_internal_gamepad_trigger_axis(padIndex, axisIndex, normalize);

	if (debugAxisInput) logAxis(axisIndex);
}

void Kore::HIDGamepad::inputValueCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef inIOHIDValueRef) {}

void Kore::HIDGamepad::valueAvailableCallback(void* inContext, IOReturn inResult, void* inSender) {
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

const char *kinc_gamepad_vendor(int gamepad) {
	return "unknown";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "unknown";
}
