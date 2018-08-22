#pragma once

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

namespace Kore {
	class HIDGamepad {
	private:
		static void inputValueCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef inIOHIDValueRef);
		static void valueAvailableCallback(void* inContext, IOReturn inResult, void* inSender);

		void reset();

		void initDeviceElements(CFArrayRef elements);

		void buttonChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex);
		void axisChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex);

		int padIndex;
		IOHIDDeviceRef hidDeviceRef;
		IOHIDQueueRef hidQueueRef;
		int hidDeviceVendorID;
		int hidDeviceProductID;
		char hidDeviceVendor[64];
		char hidDeviceProduct[64];

		IOHIDElementCookie axis[6];
		IOHIDElementCookie buttons[15];

	public:
		HIDGamepad();
		~HIDGamepad();

		void bind(IOHIDDeviceRef deviceRef, int padIndex);
		void unbind();
	};
}
