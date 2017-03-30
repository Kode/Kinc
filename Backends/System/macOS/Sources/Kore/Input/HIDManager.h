#pragma once

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

#include "HIDGamepad.h"

namespace Kore {
	class HIDManager {

	private:
		IOHIDManagerRef managerRef;

		int initHIDManager();

		bool addMatchingArray(CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef);
		CFMutableDictionaryRef createDeviceMatchingDictionary(u32 inUsagePage, u32 inUsage);

		static void deviceConnected(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
		static void deviceRemoved(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);

	public:
		HIDManager();
		~HIDManager();

		static void cleanupPad(int index);
	};
}
