#pragma once


#include <vector>
#include <string>
#include <map>
#include <list>

#include <CoreFoundation/CoreFoundation.h>
#import <CoreFoundation/CFString.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOCFPlugIn.h>
#import <IOKit/hid/IOHIDLib.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>

namespace Kore
{
    class MacJoyStick;

    //! Map of device objects connected and their respective vendors
    typedef std::vector<std::string> DeviceList;
    
    /**
     Simple wrapper class for CFString which will create a valid CFString and retain ownership until class instance is outof scope
     To Access the CFStringRef instance, simply cast to void*, pass into a function expecting a void* CFStringRef object, or access via cf_str() method
     */
    class Kore_CFString
    {
    public:
        Kore_CFString() { m_StringRef = CFStringCreateWithCString(NULL, "", kCFStringEncodingUTF8); }
        Kore_CFString(const char* c_str) { m_StringRef = CFStringCreateWithCString(NULL, c_str, kCFStringEncodingUTF8); }
        Kore_CFString(const std::string &s_str) { m_StringRef = CFStringCreateWithCString(NULL, s_str.c_str(), kCFStringEncodingUTF8); }
        ~Kore_CFString() { CFRelease(m_StringRef); }
        
        //Allow this class to be autoconverted to base class of StringRef (void*)
        operator void*() { return (void*)m_StringRef; }
        CFStringRef cf_str() { return m_StringRef; }
        
    private:
        CFStringRef m_StringRef;
    };
    
	//Information needed to create Mac HID Devices
	class HidInfo
	{
	public:
		HidInfo() : numButtons(0), numHats(0), numAxes(0), inUse(false), interface(0)
		{
		}

		//Useful tracking information
		std::string vendor;
		std::string productKey;
		std::string combinedKey;

		//Retain some count information for recreating devices without having to reparse
		int numButtons;
		int numHats;
		int numAxes;
		bool inUse;

		//Used for opening a read/write/tracking interface to device
		IOHIDDeviceInterface **interface;
	};

	typedef std::vector<HidInfo*> HidInfoList;
		
	class MacHIDManager
	{
	public:
		MacHIDManager();
		~MacHIDManager();

		void initialize();
		
		void iterateAndOpenDevices(io_iterator_t iterator);
		io_iterator_t lookUpDevices(int usage, int page);

        /**
         @remarks Return a list of all unused devices the factory maintains
         */
        DeviceList freeDeviceList();

        /**
         @remarks Number of devices
         */
        int totalDevices();

        /**
         @remarks Number of free devices
         */
        int freeDevices();

        /**
         @remarks Does a device exist with the given vendor name
         @param vendor Vendor name to test
         */
        bool vendorExist(const std::string & vendor);

        /**
         @remarks Creates the object
         @param bufferMode True to setup for buffered events
         @param vendor Create a device with the vendor name, "" means vendor name is unimportant
         */
        MacJoyStick* create(bool bufferMode, const std::string & vendor = "");

        /**
         @remarks Destroys object
         @param obj Object to destroy
         */
        void destroy(MacJoyStick* obj);
        
        /**
         @remarks Update all joysticks
         */
        void update();

	private:
		HidInfo* enumerateDeviceProperties(CFMutableDictionaryRef propertyMap);
		void parseDeviceProperties(CFDictionaryRef properties);
		void parseDevicePropertiesGroup(CFDictionaryRef properties);

		HidInfoList mDeviceList;
        std::vector<MacJoyStick*> mJoyStickList;
	};
}