#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>

namespace  Kore {
    class HIDGamepad {
    private:
        IOHIDDeviceRef deviceRef;
        IOHIDQueueRef inIOHIDQueueRef;
        
        int axisCount;
        int buttonCount;
        IOHIDElementCookie* axis;
        IOHIDElementCookie* buttons;
        
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
