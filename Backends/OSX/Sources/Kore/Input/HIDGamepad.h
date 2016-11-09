#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>

#include <Kore/Input/Gamepad.h>

namespace  Kore {
    class HIDGamepad {
    private:
        IOHIDDeviceRef deviceRef;
        IOHIDQueueRef inIOHIDQueueRef;
        
        int padIndex;
        
        void initHIDDevice();
        
        static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
        static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender);
        
        void initElementsFromArray(CFArrayRef elements);
        
    public:
        
        HIDGamepad(IOHIDDeviceRef deviceRef, int ID);
        ~HIDGamepad();
        
        // Property functions
        int getVendorID();
        int getProductID();
        char* getProductKey();
    };
}
