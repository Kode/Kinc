#pragma once

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

namespace Kore {
	class HIDGamepad {
	private:
		IOHIDDeviceRef deviceRef;
		IOHIDQueueRef inIOHIDQueueRef;
		int padIndex;

		void initHIDDevice();

		static void inputValueCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef inIOHIDValueRef);
		static void valueAvailableCallback(void* inContext, IOReturn inResult, void* inSender);
		static void deviceRemovalCallback(void* inContext, IOReturn inResult, void* inSender);

		void initElementsFromArray(CFArrayRef elements);

		void buttonChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex);
		void axisChanged(IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex);

	public:
		HIDGamepad(IOHIDDeviceRef deviceRef, int ID);
		~HIDGamepad();

		// Property functions
		int getVendorID();
		int getProductID();
		char* getProductKey();
		char* getManufacturerKey();
	};
}
