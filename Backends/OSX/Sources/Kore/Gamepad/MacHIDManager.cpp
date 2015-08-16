#include <Kore/Gamepad/MacHIDManager.h>
#include <Kore/Gamepad/MacJoyStick.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Log.h>

#include <iostream>
#include <string>

using namespace std;

using namespace Kore;

//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
template<typename T>
T getDictionaryItemAsRef(CFDictionaryRef dict, const char* keyName)
{
	return CFDictionaryGetValue(dict, Kore_CFString(keyName));
}

template<>
CFArrayRef getDictionaryItemAsRef(CFDictionaryRef dict, const char* keyName)
{
	CFTypeRef temp = CFDictionaryGetValue(dict, Kore_CFString(keyName));
	
	if(temp && CFGetTypeID(temp) == CFArrayGetTypeID())
		return (CFArrayRef)temp;
	else
		return 0;
}

template<>
CFStringRef getDictionaryItemAsRef(CFDictionaryRef dict, const char* keyName)
{
	CFTypeRef temp = CFDictionaryGetValue(dict, Kore_CFString(keyName));
	
	if(temp && CFGetTypeID(temp) == CFStringGetTypeID())
		return (CFStringRef)temp;
	else
		return 0;
}

template<>
CFNumberRef getDictionaryItemAsRef(CFDictionaryRef dict, const char* keyName)
{
	CFTypeRef temp = CFDictionaryGetValue(dict, Kore_CFString(keyName));
	
	if(temp && CFGetTypeID(temp) == CFNumberGetTypeID())
		return (CFNumberRef)temp;
	else
		return 0;
}

//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
template<typename T>
T getArrayItemAsRef(CFArrayRef array, CFIndex idx)
{
	return CFArrayGetValueAtIndex(array, idx);
}

template<>
CFDictionaryRef getArrayItemAsRef(CFArrayRef array, CFIndex idx)
{
	CFTypeRef temp = CFArrayGetValueAtIndex(array, idx);
	
	if(temp && CFGetTypeID(temp) == CFDictionaryGetTypeID())
		return (CFDictionaryRef)temp;
	else
		return 0;
}

//------------------------------------------------------------------------------------------------------//
int getInt32(CFNumberRef ref)
{
	int r = 0;
	if (r) 
		CFNumberGetValue(ref, kCFNumberIntType, &r);
	return r;
}

//--------------------------------------------------------------------------------//
MacHIDManager::MacHIDManager()
{
}

//--------------------------------------------------------------------------------//
MacHIDManager::~MacHIDManager()
{
	mJoyStickList.clear();
}

//------------------------------------------------------------------------------------------------------//
void MacHIDManager::initialize()
{
	//Make the search more specific by adding usage flags
	int usage = kHIDUsage_GD_Joystick;
	int page = kHIDPage_GenericDesktop;
	
	io_iterator_t iterator = lookUpDevices(usage, page);
	
	if(iterator)
		iterateAndOpenDevices(iterator);
	
	//Doesn't support multiple usage flags, iterate twice
	usage = kHIDUsage_GD_GamePad;
	iterator = lookUpDevices(usage, page);
	
	if(iterator)
		iterateAndOpenDevices(iterator);
	
	// Initialize all joisticks
	int joystickCount = totalDevices();
	for(int i = 0; i < joystickCount; i++) {
		MacJoyStick* joystick = create(true);

		if(joystick != NULL) {
			// Initialize the joystick
			joystick->_initialize();
			
			// Get the vendor info
            Gamepad* gamepad = Gamepad::get(joystickCount);
			size_t size = joystick->mVendor->size();
			gamepad->vendor = new char[size]();
            strcpy((char*)gamepad->vendor, joystick->mVendor->c_str());
			
			// Add the joystick to the list
			Kore::log(Kore::LogLevel::Info, "Added new joystick '%s'", (char*)joystick->mVendor->c_str());
			mJoyStickList.push_back(joystick);
		}
	}
}

//------------------------------------------------------------------------------------------------------//
io_iterator_t MacHIDManager::lookUpDevices(int usage, int page)
{
	CFMutableDictionaryRef deviceLookupMap = IOServiceMatching(kIOHIDDeviceKey);
    if(!deviceLookupMap) {
		Kore::log(Kore::LogLevel::Info, "Could not setup HID device search parameters");
    }
	
	CFNumberRef usageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
	CFNumberRef pageRef  = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
	
	CFDictionarySetValue(deviceLookupMap, CFSTR(kIOHIDPrimaryUsageKey), usageRef);
	CFDictionarySetValue(deviceLookupMap, CFSTR(kIOHIDPrimaryUsagePageKey), pageRef);
	
	//IOServiceGetMatchingServices consumes the map so we do not have to release it ourself
	io_iterator_t iterator = 0;
	IOReturn result = IOServiceGetMatchingServices(kIOMasterPortDefault, deviceLookupMap, &iterator);
	
	CFRelease(usageRef);
	CFRelease(pageRef);
	
	if(result == kIOReturnSuccess) {
		return iterator;
	} else {
		//TODO: Throw exception instead?
		return 0;
	}
}

//------------------------------------------------------------------------------------------------------//
void MacHIDManager::iterateAndOpenDevices(io_iterator_t iterator)
{
	io_object_t hidDevice = 0;
	while ((hidDevice = IOIteratorNext(iterator)) !=0)
	{
		//Get the current registry items property map
		CFMutableDictionaryRef propertyMap = 0;
		if (IORegistryEntryCreateCFProperties(hidDevice, &propertyMap, kCFAllocatorDefault, kNilOptions) == KERN_SUCCESS && propertyMap)
		{
			//Go through device to find all needed info
			HidInfo* hid = enumerateDeviceProperties(propertyMap);
			
			if(hid)
			{
				//todo - we need to hold an open interface so we do not have to enumerate again later
				//should be able to watch for device removals also
				
				// Testing opening / closing interface
				IOCFPlugInInterface **pluginInterface = NULL;
				SInt32 score = 0;
				if (IOCreatePlugInInterfaceForService(hidDevice, kIOHIDDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score) == kIOReturnSuccess)
				{
					IOHIDDeviceInterface **interface;
					
					HRESULT pluginResult = (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID), (void **)&(interface));
					
                    if(pluginResult != S_OK) {
						Kore::log(Kore::LogLevel::Info, "Not able to create plugin interface");
                    }
					
					IODestroyPlugInInterface(pluginInterface);
					
					hid->interface = interface;
					
					//Check for duplicates - some devices have multiple usage
					if(std::find(mDeviceList.begin(), mDeviceList.end(), hid) == mDeviceList.end())
						mDeviceList.push_back(hid);
				}
			}
		}
	}
	
	IOObjectRelease(iterator);
}

const char* getCString(CFStringRef cfString) {
	const char *useUTF8StringPtr = NULL;
	UInt8 *freeUTF8StringPtr = NULL;

	CFIndex stringLength = CFStringGetLength(cfString), usedBytes = 0L;

	if ((useUTF8StringPtr = CFStringGetCStringPtr(cfString,	kCFStringEncodingUTF8)) == NULL) {
		if ((freeUTF8StringPtr = (UInt8*)malloc(stringLength + 1L)) != NULL) {
			CFStringGetBytes(cfString, CFRangeMake(0L, stringLength),
					kCFStringEncodingUTF8, '?', false, freeUTF8StringPtr,
					stringLength, &usedBytes);
			freeUTF8StringPtr[usedBytes] = 0;
			useUTF8StringPtr = (const char *) freeUTF8StringPtr;
		}
	}

	return useUTF8StringPtr;
}

//------------------------------------------------------------------------------------------------------//
HidInfo* MacHIDManager::enumerateDeviceProperties(CFMutableDictionaryRef propertyMap)
{
	HidInfo* info = new HidInfo();

	CFStringRef str = getDictionaryItemAsRef<CFStringRef>(propertyMap, kIOHIDManufacturerKey);
	if (str) {
		const char* str_c = getCString(str);
		if(str_c) {
			info->vendor = str_c;
			free((char*)str_c);
		} else {
			info->vendor = "Unknown Vendor";
		}
	}

	str = getDictionaryItemAsRef<CFStringRef>(propertyMap, kIOHIDProductKey);
	if (str) {
		const char* str_c = getCString(str);
		if(str_c) {
			info->productKey = str_c;
            // mzechner: we leak this so packr packaged apps
            // works. I have no idea why this would fail in
            // case of packr. My guess it register allocation
            // and aliasing with str_c above.
			//free((char*)str_c);
		} else {
			info->productKey = "Unknown Product";
		}
	}

	info->combinedKey = info->vendor + " " + info->productKey;

	//Go through all items in this device (i.e. buttons, hats, sticks, axes, etc)
	CFArrayRef array = getDictionaryItemAsRef<CFArrayRef>(propertyMap, kIOHIDElementKey);
	if (array)
		for (int i = 0; i < CFArrayGetCount(array); i++)
			parseDeviceProperties(getArrayItemAsRef<CFDictionaryRef>(array, i));
	
	return info;
}

//------------------------------------------------------------------------------------------------------//
void MacHIDManager::parseDeviceProperties(CFDictionaryRef properties)
{
	if(!properties)
		return;
	
	CFArrayRef array = getDictionaryItemAsRef<CFArrayRef>(properties, kIOHIDElementKey);
	if (array)
	{
		for (int i = 0; i < CFArrayGetCount(array); i++)
		{
			CFDictionaryRef element = getArrayItemAsRef<CFDictionaryRef>(array, i);
			if (element)
			{
				if(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementTypeKey)) == kIOHIDElementTypeCollection) 
				{	//Check if we need to recurse further intoi another collection
					if(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementUsagePageKey)) == kHIDPage_GenericDesktop)
						parseDeviceProperties(element);
				}
				else
				{
					switch(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementUsagePageKey)))
					{
						case kHIDPage_GenericDesktop:
							switch(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementUsageKey)))
						{
							case kHIDUsage_GD_Pointer:
								cout << "\tkHIDUsage_GD_Pointer\n";
								parseDevicePropertiesGroup(element);
								break;
							case kHIDUsage_GD_X:
							case kHIDUsage_GD_Y:
							case kHIDUsage_GD_Z:
							case kHIDUsage_GD_Rx:
							case kHIDUsage_GD_Ry:
							case kHIDUsage_GD_Rz:
								cout << "\tAxis\n";
								break;
							case kHIDUsage_GD_Slider:
							case kHIDUsage_GD_Dial:
							case kHIDUsage_GD_Wheel:
								cout << "\tUnsupported kHIDUsage_GD_Wheel\n";
								break;
							case kHIDUsage_GD_Hatswitch:
								cout << "\tUnsupported - kHIDUsage_GD_Hatswitch\n";
								break;
						}
							break;
						case kHIDPage_Button:
							cout << "\tkHIDPage_Button\n";
							break;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------//
void MacHIDManager::parseDevicePropertiesGroup(CFDictionaryRef properties)
{
	if(!properties)
		return;
	
	CFArrayRef array = getDictionaryItemAsRef<CFArrayRef>(properties, kIOHIDElementKey);
	if(array)
	{
		for (int i = 0; i < CFArrayGetCount(array); i++)
		{
			CFDictionaryRef element = getArrayItemAsRef<CFDictionaryRef>(array, i);
			if (element)
			{
				switch(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementUsagePageKey)))
				{
					case kHIDPage_GenericDesktop:
						switch(getInt32(getDictionaryItemAsRef<CFNumberRef>(element, kIOHIDElementUsageKey)))
					{
						case kHIDUsage_GD_X:
						case kHIDUsage_GD_Y:
						case kHIDUsage_GD_Z:
						case kHIDUsage_GD_Rx:
						case kHIDUsage_GD_Ry:
						case kHIDUsage_GD_Rz:
							cout << "\t\tAxis\n";
							break;
						case kHIDUsage_GD_Slider:
						case kHIDUsage_GD_Dial:
						case kHIDUsage_GD_Wheel:
							cout << "\tUnsupported - kHIDUsage_GD_Wheel\n";
							break;
						case kHIDUsage_GD_Hatswitch:
							cout << "\tUnsupported - kHIDUsage_GD_Hatswitch\n";
							break;
					}
						break;
					case kHIDPage_Button:
						break;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------//
DeviceList MacHIDManager::freeDeviceList()
{
	DeviceList ret;
	HidInfoList::iterator it = mDeviceList.begin(), end = mDeviceList.end();
	for(; it != end; ++it)
	{
		if((*it)->inUse == false)
			ret.push_back((*it)->combinedKey);
	}
	
	return ret;
}

//--------------------------------------------------------------------------------//
int MacHIDManager::totalDevices()
{
	return (int)mDeviceList.size();
}

//--------------------------------------------------------------------------------//
int MacHIDManager::freeDevices()
{
	int ret = 0;
	HidInfoList::iterator it = mDeviceList.begin(), end = mDeviceList.end();
	
	for(; it != end; ++it)
	{
		if((*it)->inUse == false)
			ret++;
	}
	
	return ret;
}

//--------------------------------------------------------------------------------//
bool MacHIDManager::vendorExist(const std::string & vendor)
{
	HidInfoList::iterator it = mDeviceList.begin(), end = mDeviceList.end();
	
	for(; it != end; ++it)
	{
		if((*it)->combinedKey == vendor)
			return true;
	}
	
	return false;
}

//--------------------------------------------------------------------------------//
MacJoyStick* MacHIDManager::create(bool bufferMode, const std::string & vendor)
{
	MacJoyStick* obj = 0;
	
	HidInfoList::iterator it = mDeviceList.begin(), end = mDeviceList.end();
	for(; it != end; ++it)
	{
		if((*it)->inUse == false && (vendor == "" || (*it)->combinedKey == vendor))
		{
			int totalDevs = totalDevices();
			int freeDevs = freeDevices();
			int devID = totalDevs - freeDevs;

			obj = new MacJoyStick((*it)->combinedKey, bufferMode, *it, devID);
			(*it)->inUse = true;
			return obj;
        }
	}
	
	return obj;
}

//--------------------------------------------------------------------------------//
void MacHIDManager::destroy(MacJoyStick* obj)
{
	delete obj;
}

//--------------------------------------------------------------------------------//
void MacHIDManager::update()
{
	for(int i = 0; i < mJoyStickList.size(); i++) {
		mJoyStickList[i]->capture();
	}
}

