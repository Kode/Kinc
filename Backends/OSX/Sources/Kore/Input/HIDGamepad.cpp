#include "HIDGamepad.h"

using namespace Kore;

HIDGamepad::HIDGamepad(IOHIDDeviceRef deviceRef) : deviceRef(deviceRef), elementCFArrayRef(NULL) {
    initHIDDevice();
}

HIDGamepad::~HIDGamepad() {
    if (deviceRef) {
        IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDDeviceRegisterRemovalCallback(deviceRef, nullptr, this);
    }
    
    if (inIOHIDQueueRef) {
        IOHIDQueueStop(inIOHIDQueueRef);
    }

}

void HIDGamepad::initHIDDevice() {
    if(deviceRef) {
        
        // Get all elements for a specific device
        elementCFArrayRef = IOHIDDeviceCopyMatchingElements(deviceRef, NULL, kIOHIDOptionsTypeNone);
        
        
        // Get elements
        /*CFSetRef device_set = IOHIDManagerCopyDevices(managerRef);
        CFIndex num_devices = CFSetGetCount(device_set);
        log(Info, "%d devices found\n",(int)num_devices);

        
        
        CFIndex nElem = CFSetGetCount(elementCFArrayRef);
        //printf("HID Device %d has %d elements.\n",i,nElem);
        
        int j;
        for(j=0; j<nElem; j++)
        {
            IOHIDElementRef elem=(IOHIDElementRef)CFArrayGetValueAtIndex(elemAry,j);
            IOHIDElementType tType = IOHIDElementGetType(elementRef);
            
        }*/
        
        
        // Open device
        IOReturn ret = IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeSeizeDevice);
        
        // Register routines
        IOHIDDeviceRegisterInputValueCallback(deviceRef, inputValueCallback, this);
        
        IOHIDDeviceScheduleWithRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        
        // Create a queue to access the values
        inIOHIDQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, deviceRef, 32, kIOHIDOptionsTypeNone);
        if (CFGetTypeID(inIOHIDQueueRef) == IOHIDQueueGetTypeID()) {
            // this is a valid HID queue reference!
            printf("Valid HID queue reference\n");
            initElementsFromArray(elementCFArrayRef);
            IOHIDQueueStart(inIOHIDQueueRef);
        }
        
    }
}

void HIDGamepad::initElementsFromArray(CFArrayRef elements) {
    for (CFIndex i = 0, count = CFArrayGetCount(elements); i < count; ++i) {
        IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        IOHIDElementType elemType = IOHIDElementGetType(element);
        
        switch(elemType) {
            case kIOHIDElementTypeInput_Button:
                //printf(" Button    ");
                if(!IOHIDQueueContainsElement(inIOHIDQueueRef, element))
                    IOHIDQueueAddElement(inIOHIDQueueRef, element);
                break;
            case kIOHIDElementTypeInput_Axis:
                //printf(" Axis      ");
                if(!IOHIDQueueContainsElement(inIOHIDQueueRef, element))
                    IOHIDQueueAddElement(inIOHIDQueueRef, element);
                break;
            default:
                break;
        }
            
    }
}

// Function to get a long device property
// Returns FALSE if the property isn't found or can't be converted to a long
Boolean HIDGamepad::getLongProperty(IOHIDDeviceRef inDeviceRef, CFStringRef inKey, long * outValue) {
    Boolean result = FALSE;
    CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(inDeviceRef, inKey);
    if (tCFTypeRef) {
        if (CFNumberGetTypeID() == CFGetTypeID(tCFTypeRef)) {
            result = CFNumberGetValue((CFNumberRef) tCFTypeRef, kCFNumberSInt32Type, outValue);
        }
    }
    return result;
}

// Get a HID device's vendor ID (long)
long HIDGamepad::getVendorID() {
    long result = 0;
    (void) getLongProperty(deviceRef, CFSTR(kIOHIDVendorIDKey), &result);
    return result;
}

// Get a HID device's product ID (long)
long HIDGamepad::getProductID() {
    long result = 0;
    (void) getLongProperty(deviceRef, CFSTR(kIOHIDProductIDKey), &result);
    return result;
}

void HIDGamepad::inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef) {
    printf("%s(context: %p, result: %p, sender: %p, value: %p).\n",
           __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender, (void*) inIOHIDValueRef);
}

void HIDGamepad::getValue() {

    IOHIDValueRef valueRef = IOHIDQueueCopyNextValue(inIOHIDQueueRef);
    
    double scaled = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
    int value = IOHIDValueGetIntegerValue(valueRef);
    
    printf(" %5d",value);
    
    //Gamepad::_axis(<#int axis#>, <#float value#>)
    //Gamepad::_button(<#int button#>, <#float value#>)
}

