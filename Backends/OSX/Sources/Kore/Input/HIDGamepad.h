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
        
        Gamepad* gamepad;
        int mDevID;
        
        int ID;
        
        static void inputValueCallback(void *          inContext,      // context from IOHIDDeviceRegisterInputValueCallback
                                       IOReturn        inResult,       // completion result for the input value operation
                                       void *          inSender,       // IOHIDDeviceRef of the device this element is from
                                       IOHIDValueRef   inIOHIDValueRef // the new element value
        );
        
        static void valueAvailableCallback(void *   inContext, // context from IOHIDQueueRegisterValueAvailableCallback
                                           IOReturn inResult,  // the inResult
                                           void *   inSender  // IOHIDQueueRef of the queue
        );
        
        void initElementsFromArray(CFArrayRef elements);
        
    public:
        
        HIDGamepad(IOHIDDeviceRef deviceRef);
        ~HIDGamepad();
        
        void initHIDDevice();
        
        // Property functions
        int getVendorID();
        int getProductID();
        char* getProductKey();
    };
}
