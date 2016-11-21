#include "HIDGamepad.h"

#include <Kore/Input/Gamepad.h>
#include <Kore/Log.h>

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
    
    // TODO: make it
    char* toString(int number) {
        static char buffer[16];
        sprintf(buffer, "%i", number);
        return buffer;
    }
}

HIDGamepad::HIDGamepad(IOHIDDeviceRef deviceRef, int padIndex) : deviceRef(deviceRef), padIndex(padIndex) {
    initHIDDevice();
    
    Gamepad* gamepad = Gamepad::get(padIndex);
    
    gamepad->vendor = toString(getVendorID());
    gamepad->productName = getProductKey();
}

HIDGamepad::~HIDGamepad() {
    if (deviceRef) {
        IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDDeviceRegisterRemovalCallback(deviceRef, NULL, this);
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
        IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        
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
    HIDGamepad* pad = (HIDGamepad*)inContext;
    //log(Info, "%s(context: %p, result: %p, sender: %p).\n",
    //       __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender);
    do {
        IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef) inSender, 0.);
        if (!valueRef) break;
        // process the HID value reference
    
        IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);
        IOHIDElementType elemType = IOHIDElementGetType(elementRef);
        
        //log(Info, "Type %d %d\n", elemType, elementRef);
        switch(elemType) {
            case kIOHIDElementTypeInput_Button: {
                IOHIDElementCookie button = IOHIDElementGetCookie(elementRef);
                
                //double rawValue = IOHIDValueGetIntegerValue(valueRef);
                double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
                
                // Buttons normalize to the range [0.0, 1.0] (0 - release, 1 - pressed)
                double min = IOHIDElementGetLogicalMin(elementRef);
                double max = IOHIDElementGetLogicalMax(elementRef);
                double normalize = (rawValue - min) / (max - min);
                
                //log(Info, "%f %f %f %f", rawValue, min, max, normalize);
                
                Gamepad::get(pad->padIndex)->_button((int)button, normalize);
                
                break;
            }
            case kIOHIDElementTypeInput_Misc:
            case kIOHIDElementTypeInput_Axis: {
                IOHIDElementCookie axis = IOHIDElementGetCookie(elementRef);
                
                //if (axis < 20 || axis > 24) // TODO: this works only on joystick PS3
                //    break;
                
                //double rawValue = IOHIDValueGetIntegerValue(valueRef);
                double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
                
                // Axes normalize to the range [-1.0, 1.0] (-1 - left, 0 - release, 1 - right)
                double min = IOHIDElementGetPhysicalMin(elementRef);
                double max = IOHIDElementGetPhysicalMax(elementRef);
                double normalize = (((rawValue - min) / (max - min)) * 2) - 1;
                //double normalize = (rawValue - min) / (max - min);
                //if (axis % 2 == 1)
                //    normalize = -normalize;
                
                //log(Info, "%f %f %f %f", rawValue, min, max, normalize);
                
                Gamepad::get(pad->padIndex)->_axis(axis, normalize);
                
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
        
        CFRelease(valueRef);
    } while (1) ;
    
}

