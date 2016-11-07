#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>

namespace  Kore {
    class HIDGamepad {
    private:
        IOHIDDeviceRef deviceRef;
        CFArrayRef elementCFArrayRef;
        IOHIDQueueRef inIOHIDQueueRef;
        
        static void inputValueCallback(void *          inContext,      // context from IOHIDDeviceRegisterInputValueCallback
                                       IOReturn        inResult,       // completion result for the input value operation
                                       void *          inSender,       // IOHIDDeviceRef of the device this element is from
                                       IOHIDValueRef   inIOHIDValueRef // the new element value
        );
        
        Boolean getLongProperty(IOHIDDeviceRef inDeviceRef,     // the HID device reference
                                CFStringRef inKey,              // the kIOHIDDevice key (as a CFString)
                                long * outValue);               // address where to return the output value

        void initElementsFromArray(CFArrayRef elements);
        
    public:
        
        HIDGamepad(IOHIDDeviceRef deviceRef);
        ~HIDGamepad();
        
        void initHIDDevice();
        
        // Property functions
        long getVendorID();
        long getProductID();
        
        void getValue();
    
    };
}
