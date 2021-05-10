#pragma once

#include <Kore/global.h>

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

		// Slots to hold details on connected devices
		struct DeviceRecord {
			bool 			connected	= false;
			IOHIDDeviceRef 	device		= nullptr;
			HIDGamepad 		pad;
		};
		DeviceRecord* devices = new DeviceRecord[MAX_DEVICES];

	public:
		// Maximum number of devices supported
		// Corresponds to size of Kore::Gamepad array
		static const int MAX_DEVICES = 12;

		HIDManager();
		~HIDManager();
	};
}
