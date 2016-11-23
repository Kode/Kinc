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
    
    // TODO: make it better
    char* toString(int number) {
        static char buffer[16];
        sprintf(buffer, "%i", number);
        return buffer;
    }
}

HIDGamepad::HIDGamepad(IOHIDDeviceRef deviceRef, int padIndex) : deviceRef(deviceRef), padIndex(padIndex), axisCount(6), buttonCount(15), invertY(true) {
    axis = new IOHIDElementCookie[axisCount];
    buttons = new IOHIDElementCookie[buttonCount];
    initHIDDevice();
    
    Gamepad* gamepad = Gamepad::get(padIndex);
    
    gamepad->vendor = toString(getVendorID());
    gamepad->productName = getProductKey();
    log(Info, "Add gamepad: Vendor: %s, Name: %s", gamepad->vendor, gamepad->productName);
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

    delete axis;
    delete buttons;
    axis = nullptr;
    buttons = nullptr;
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
        IOHIDElementRef elementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        IOHIDElementType elemType = IOHIDElementGetType(elementRef);
        
        IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);
        
        uint32_t page = IOHIDElementGetUsagePage(elementRef);
        uint32_t usage = IOHIDElementGetUsage(elementRef);
        
        // Match up items
        switch(page) {
            case 1:  // Generic Desktop
                switch(usage) {
                    case 53:  // Right trigger
                        log(Info, "Right trigger axis[5] = %i", cookie);
                        axis[5] = cookie;
                        break;
                    case 50:  // Left trigger
                        log(Info, "Left trigger axis[4] = %i", cookie);
                        axis[4] = cookie;
                        break;
                    case 52:  // Right stick Y
                        log(Info, "Right stick Y axis[3] = %i", cookie);
                        axis[3] = cookie;
                        break;
                    case 51:  // Right stick X
                        log(Info, "Right stick X axis[2] = %i", cookie);
                        axis[2] = cookie;
                        break;
                    case 49:  // Left stick Y
                        log(Info, "Left stick Y axis[1] = %i", cookie);
                        axis[1] = cookie;
                        break;
                    case 48:  // Left stick X
                        log(Info, "Left stick X axis[0] = %i", cookie);
                        axis[0] = cookie;
                        break;
                    default:
                        break;
                }
                break;
            case 9:  // Button
                if((usage >= 1) && (usage <= 15)) {
                    // Button 1-11
                    buttons[usage-1] = cookie;
                    log(Info, "Button %i = %i", usage-1, cookie);
                }
                break;
            default:
                break;
        }
        
        
        if (elemType == kIOHIDElementTypeInput_Misc || elemType == kIOHIDElementTypeInput_Button || elemType == kIOHIDElementTypeInput_Axis) {
            if(!IOHIDQueueContainsElement(inIOHIDQueueRef, elementRef))
                IOHIDQueueAddElement(inIOHIDQueueRef, elementRef);
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
    
    IOHIDElementRef elementRef = IOHIDValueGetElement(inIOHIDValueRef);
    IOHIDElementType elemType = IOHIDElementGetType(elementRef);
    
    IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);
    
    // for Xbox 360 wired controller
    uint32_t page = IOHIDElementGetUsagePage(elementRef);
    uint32_t usage = IOHIDElementGetUsage(elementRef);
    
    log(Info, "page %i, usage %i", page, usage);
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
                IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);
                
                bool found = false;
                int button = 0;
                for (int i = 0; i < pad->buttonCount; ++i) {
                    if (cookie == pad->buttons[i]) {
                        button = i;
                        found = true;
                    }
                }
                
                if (!found) {
                    break;
                }
                
                //double rawValue = IOHIDValueGetIntegerValue(valueRef);
                double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
                
                // Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
                double min = IOHIDElementGetLogicalMin(elementRef);
                double max = IOHIDElementGetLogicalMax(elementRef);
                double normalize = (rawValue - min) / (max - min);
                
                //log(Info, "%f %f %f %f", rawValue, min, max, normalize);
                
                Gamepad::get(pad->padIndex)->_button((int)button, normalize);
                
                break;
            }
            case kIOHIDElementTypeInput_Misc:
            case kIOHIDElementTypeInput_Axis: {
                IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);
                
                bool found = false;
                int axis = 0;
                for (int i = 0; i < pad->axisCount; ++i) {
                    if (cookie == pad->axis[i]) {
                        axis = i;
                        found = true;
                    }
                }
                
                if (!found) {
                    break;
                }
                
                //double rawValue = IOHIDValueGetIntegerValue(valueRef);
                double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
                
                // Normalize axis value to the range [-1.0, 1.0] (e.g. -1 - left, 0 - release, 1 - right)
                double min = IOHIDElementGetPhysicalMin(elementRef);
                double max = IOHIDElementGetPhysicalMax(elementRef);
                double normalize = (((rawValue - min) / (max - min)) * 2) - 1;
                //double normalize = (rawValue - min) / (max - min);
                //if (axis % 2 == 1)
                //    normalize = -normalize;
                
                // Invert Y axis
                if (pad->invertY && (axis == 1 || axis == 3))
                    normalize = -normalize;
                
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


