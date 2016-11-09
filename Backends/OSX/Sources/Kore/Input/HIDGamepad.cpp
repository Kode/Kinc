#include "HIDGamepad.h"

#include <Kore/Log.h>

#include <stdio.h>
#include <stdlib.h>

using namespace Kore;

namespace {
    char* toString(CFStringRef string) {
        if (string == NULL) {
            return NULL;
        }
        
        CFIndex length = CFStringGetLength(string);
        CFIndex maxSize =
        CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
        char *buffer = (char *)malloc(maxSize);
        if (CFStringGetCString(string, buffer, maxSize, kCFStringEncodingUTF8)) {
            return buffer;
        }
        return NULL;
    }
}

//TODO: set ID
HIDGamepad::HIDGamepad(IOHIDDeviceRef deviceRef) : deviceRef(deviceRef), mDevID(0) {
    initHIDDevice();
    
    gamepad = new Gamepad();
    //gamepad->vendor = getVendorID();
    gamepad->productName = getProductKey();
}

HIDGamepad::~HIDGamepad() {
    if (deviceRef) {
        IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDDeviceRegisterRemovalCallback(deviceRef, nullptr, this);
    }
    
    if (inIOHIDQueueRef) {
        IOHIDQueueStop(inIOHIDQueueRef);
        IOHIDQueueUnscheduleFromRunLoop(inIOHIDQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }

}

void HIDGamepad::initHIDDevice() {
    if(deviceRef) {
        
        // Get all elements for a specific device
        CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(deviceRef, NULL, kIOHIDOptionsTypeNone);
        
        // Open device
        IOReturn ret = IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        
        // Register routines
        IOHIDDeviceRegisterInputValueCallback(deviceRef, inputValueCallback, this);
        
        IOHIDDeviceScheduleWithRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        
        // Create a queue to access the values
        inIOHIDQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, deviceRef, 32, kIOHIDOptionsTypeNone);
        if (CFGetTypeID(inIOHIDQueueRef) == IOHIDQueueGetTypeID()) {
            // this is a valid HID queue reference!
            initElementsFromArray(elementCFArrayRef);
            IOHIDQueueStart(inIOHIDQueueRef);
            IOHIDQueueRegisterValueAvailableCallback(inIOHIDQueueRef, valueAvailableCallback, this);
            IOHIDQueueScheduleWithRunLoop(inIOHIDQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        }
        
    }
}

void HIDGamepad::initElementsFromArray(CFArrayRef elements) {
    for (CFIndex i = 0, count = CFArrayGetCount(elements); i < count; ++i) {
        IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        IOHIDElementType elemType = IOHIDElementGetType(element);
        
        if (elemType == kIOHIDElementTypeInput_Misc || elemType == kIOHIDElementTypeInput_Button || elemType == kIOHIDElementTypeInput_Axis) {
            if(!IOHIDQueueContainsElement(inIOHIDQueueRef, element))
                IOHIDQueueAddElement(inIOHIDQueueRef, element);
        }
    }
}

// Get a HID device's vendor ID (long)
int HIDGamepad::getVendorID() {
    int vendorID = 0;
    CFNumberRef cfVendorID = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDVendorIDKey));
    CFNumberGetValue(cfVendorID, kCFNumberIntType, &vendorID);
    return vendorID;
}

// Get a HID device's product ID (long)
int HIDGamepad::getProductID() {
    int productID = 0;
    CFNumberRef cfVendorID = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDProductIDKey));
    CFNumberGetValue(cfVendorID, kCFNumberIntType, &productID);
    return productID;
}

// Get a HID device's product key (string)
char* HIDGamepad::getProductKey() {
    CFStringRef productName = (CFStringRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDProductKey));
    return toString(productName);
}

void HIDGamepad::inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef) {
    //log(Info, "%s(context: %p, result: %p, sender: %p, value: %p).\n",
    //    __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender, (void*) inIOHIDValueRef);
}

void HIDGamepad::valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender) {
    //log(Info, "%s(context: %p, result: %p, sender: %p).\n",
    //       __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender);
    do {
        IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef) inSender, 0.);
        if (!valueRef) break;
        // process the HID value reference
    
        IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);
        IOHIDElementType elemType = IOHIDElementGetType(elementRef);
        
        //log(Info, "Type %d %d\n", elemType, elementRef);
        //log(Info, "logicalMin %d logicalMax %d physicalMin %d physicalMax %d \n", logicalMin, logicalMax, physicalMin, physicalMax);
        switch(elemType) {
            case kIOHIDElementTypeInput_Button: {
                IOHIDElementCookie button = IOHIDElementGetCookie(elementRef);
                
                double rawValue = IOHIDValueGetIntegerValue(valueRef);
                
                // Buttons normalize to the range (0.0) - (1.0)
                CFIndex min = IOHIDElementGetLogicalMin(elementRef);
                CFIndex max = IOHIDElementGetLogicalMax(elementRef);
                double normalize = (rawValue - min) / (max - min);
                
                Gamepad::get(0)->_button((int)button, (float)normalize); // TODO: get the right pad ID
                
                break;
            }
            case kIOHIDElementTypeInput_Misc:
            case kIOHIDElementTypeInput_Axis: {
                IOHIDElementCookie axis = IOHIDElementGetCookie(elementRef);
                
                double rawValue = IOHIDValueGetIntegerValue(valueRef);
                
                // Axes normalize to the range (-1.0) - (1.0)
                CFIndex min = IOHIDElementGetPhysicalMin(elementRef);
                CFIndex max = IOHIDElementGetPhysicalMax(elementRef);
                double normalize = (((rawValue - min) / (max - min)) * 2) - 1;
                
                if (axis % 2 == 1)
                    normalize = -normalize;
                
                log(Info, "Axis %d value %f %f\n", (int)axis, rawValue, normalize);
                Gamepad::get(0)->_axis(axis, (float)normalize); // TODO: get the right pad ID
                
                
                break;
            }
            case kIOHIDElementTypeInput_ScanCodes:
                break;
            case kIOHIDElementTypeOutput:
                break;
            case kIOHIDElementTypeFeature:
                break;
            case kIOHIDElementTypeCollection:
                break;
        }
        
        
        CFRelease(valueRef);    // Don't forget to release our HID value reference
    } while (1) ;
    
}

