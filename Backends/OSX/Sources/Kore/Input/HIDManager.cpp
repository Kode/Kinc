#include "../pch.h"

#include <Kore/Input/HIDManager.h>
#include <Kore/Input/Gamepad.h>

#include <Kore/Log.h>

using namespace Kore;

HIDManager::HIDManager() : managerRef(0x0) {
    gamepadList = new HIDGamepad*[2]; // TODO:
    initHIDManager();
}

HIDManager::~HIDManager() {
    
    if (managerRef) {
        IOHIDManagerClose(managerRef, kIOHIDOptionsTypeNone);
        IOHIDManagerUnscheduleFromRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        // Unregister
        //IOHIDManagerRegisterInputValueCallback(managerRef, NULL, this);
        //IOHIDManagerRegisterDeviceMatchingCallback(managerRef, NULL, this);
        //IOHIDManagerRegisterDeviceRemovalCallback(managerRef, NULL, this);
    }
}

int HIDManager::initHIDManager() {
    
    // Initialize the IOHIDManager
    managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (CFGetTypeID(managerRef) == IOHIDManagerGetTypeID()) {
        
        // Create a matching dictionary for gamepads and joysticks
        CFMutableArrayRef matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        if (matchingCFArrayRef) {
            // Create a device matching dictionary for joysticks
            CFDictionaryRef matchingCFDictRef = createDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
            if (matchingCFDictRef) {
                // Add it to the matching array
                CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
                CFRelease(matchingCFDictRef); // and release it
            } else {
                log(Error, "%s: CreateDeviceMatchingDictionary(joystick) failed.", __PRETTY_FUNCTION__);
            }
            
            // Create a device matching dictionary for game pads
            matchingCFDictRef = createDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
            if (matchingCFDictRef) {
                // Add it to the matching array
                CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
                CFRelease(matchingCFDictRef); // and release it
            } else {
                log(Error, "%s: hu_CreateDeviceMatchingDictionary(gamepad) failed.", __PRETTY_FUNCTION__);
            }
        } else {
            log(Error, "%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__);
        }
        
        // Set the HID device matching array
        IOHIDManagerSetDeviceMatchingMultiple(managerRef, matchingCFArrayRef);
        CFRelease(matchingCFArrayRef);
        
        // Open manager
        /*IOReturn tIOReturn =*/ IOHIDManagerOpen(managerRef, kIOHIDOptionsTypeNone);
        
        // Register routines to be called when (matching) devices are connected or disconnected
        IOHIDManagerRegisterDeviceMatchingCallback(managerRef, deviceConnected, this);
        IOHIDManagerRegisterDeviceRemovalCallback(managerRef, deviceRemoved, this);
        
        IOHIDManagerScheduleWithRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        // Check how many devices are connected
        CFSetRef deviceSetRef = IOHIDManagerCopyDevices(managerRef);
        if (deviceSetRef) {
            CFIndex num_devices = CFSetGetCount(deviceSetRef);
            log(Info, "%d gamepad(s) found\n",(int)num_devices);
            CFRelease(deviceSetRef);
        }

        return 0;
    }
    

    return -1;
}

// Create matching dictionary
CFMutableDictionaryRef HIDManager::createDeviceMatchingDictionary(u32 inUsagePage, u32 inUsage) {
    // Create a dictionary to add usage page/usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (result) {
        if (inUsagePage) {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
            if (pageCFNumberRef) {
                CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
                CFRelease(pageCFNumberRef);
                
                // note: the usage is only valid if the usage page is also defined
                if (inUsage) {
                    CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
                    if (usageCFNumberRef) {
                        CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
                        CFRelease(usageCFNumberRef);
                    } else {
                        log(Error, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
                    }
                }
            } else {
                log(Error, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
            }
        }
    } else {
        log(Error, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
    }
    return result;
}

// This will be called when the HID Manager matches a new (hot plugged) HID device
void HIDManager::deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
    log(Info, "HID device plugged");
    
    HIDGamepad* hidDevice = new HIDGamepad(inIOHIDDeviceRef);
    // TODO: create a list of all gamepads
    //addNewDevice(hidDevice);
}

void HIDManager::addNewDevice(HIDGamepad hidDevice) {
    gamepadList[0] = &hidDevice;
}

// This will be called when a HID device is removed (unplugged)
void HIDManager::deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
    log(Info, "HID device removed");
}

