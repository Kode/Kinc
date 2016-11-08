#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>

#include "HIDGamepad.h"

namespace Kore {
    class HIDManager {
    
    private:
        
        IOHIDManagerRef managerRef;
        
        HIDGamepad** gamepadList;
        
        int initHIDManager();
        
        CFMutableDictionaryRef createDeviceMatchingDictionary(u32 inUsagePage, u32 inUsage);
        static void deviceConnected(void *          inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                    IOReturn        inResult,        // the result of the matching operation
                                    void *          inSender,        // the IOHIDManagerRef for the new device
                                    IOHIDDeviceRef  inIOHIDDeviceRef // the new HID device
        );
        static void deviceRemoved(void *         inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                  IOReturn       inResult,        // the result of the removing operation
                                  void *         inSender,        // the IOHIDManagerRef for the device being removed
                                  IOHIDDeviceRef inIOHIDDeviceRef // the removed HID device
        );
        
        void addNewDevice(HIDGamepad hidDevice);
        
        
    public:
        HIDManager();
        ~HIDManager();
    };
}
