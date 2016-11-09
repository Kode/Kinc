#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>

#include "HIDGamepad.h"

namespace Kore {
    class HIDManager {
    
    private:
        IOHIDManagerRef managerRef;
        
        int initHIDManager();
        
        int gamepadsCounter;
        
        CFMutableDictionaryRef createDeviceMatchingDictionary(u32 inUsagePage, u32 inUsage);
        static void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
        static void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
        
    public:
        HIDManager();
        ~HIDManager();
    };
}
