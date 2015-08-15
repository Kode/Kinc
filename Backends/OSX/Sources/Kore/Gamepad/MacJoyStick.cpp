#include <Kore/Gamepad/MacJoyStick.h>
#include <Kore/Gamepad/MacHIDManager.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Log.h>

#include <cassert>

using namespace Kore;

//--------------------------------------------------------------------------------------------------//
MacJoyStick::MacJoyStick(const std::string &vendor, bool buffered, HidInfo* info, int devID) :
mBuffered(buffered), mDevID(devID), mInfo(info)
{
    mVendor =  new std::string(vendor);
}

//--------------------------------------------------------------------------------------------------//
MacJoyStick::~MacJoyStick()
{
	//TODO: check if the queue has been started first?
	//(*mQueue)->stop(mQueue); 
	(*mQueue)->dispose(mQueue); 
	(*mQueue)->Release(mQueue); 
	
    if(mVendor != nullptr) {
        delete mVendor;
        mVendor = nullptr;
    }
	
	//TODO: check if the interface has been opened first?
	(*mInfo->interface)->close(mInfo->interface);
	(*mInfo->interface)->Release(mInfo->interface); 
}

//--------------------------------------------------------------------------------------------------//
void MacJoyStick::_initialize()
{
	assert(mInfo != 0);
	assert(mInfo->interface != 0);
	
	//TODO: Is this necessary?
	//Clear old state
	mState.mAxes.clear();
	
    if ((*mInfo->interface)->open(mInfo->interface, 0) != KERN_SUCCESS) {
        Kore::log(Kore::LogLevel::Info, "MacJoyStick::_initialize() >> Could not initialize joy device!");
        return;
    }
    
	mState.clear();
	
	_enumerateCookies();
	
	mState.mButtons.resize(mInfo->numButtons);
	mState.mAxes.resize(mInfo->numAxes);
	
    mQueue = _createQueue();
}

class FindAxisCookie : public std::unary_function<std::pair<IOHIDElementCookie, AxisInfo>&, bool>
{
public:
	FindAxisCookie(IOHIDElementCookie cookie) : m_Cookie(cookie) {}
	bool operator()(const std::pair<IOHIDElementCookie, AxisInfo>& pair) const
	{
		return pair.first == m_Cookie;
	}
private:
	IOHIDElementCookie m_Cookie;
};

//--------------------------------------------------------------------------------------------------//
void MacJoyStick::capture()
{
 	assert(mQueue != 0);
	
	AbsoluteTime zeroTime = {0,0}; 
	
	IOHIDEventStruct event; 
	IOReturn result = (*mQueue)->getNextEvent(mQueue, &event, zeroTime, 0);
    
	while(result == kIOReturnSuccess)
	{
		switch(event.type)
		{
			case kIOHIDElementTypeInput_Button:
			{
				std::vector<IOHIDElementCookie>::iterator buttonIt = std::find(mCookies.buttonCookies.begin(), mCookies.buttonCookies.end(), event.elementCookie);
				int button = (int)std::distance(mCookies.buttonCookies.begin(), buttonIt);
				mState.mButtons[button] = (event.value == 1);
				
                if(mBuffered && Gamepad::get(mDevID)->Button != nullptr)
				{
					if(event.value == 1)
						Gamepad::get(mDevID)->Button(button, 1.0f);
					else if(event.value == 0)
                        Gamepad::get(mDevID)->Button(button, 0.0f);
				}
				break;
			}
			case kIOHIDElementTypeInput_Misc:
				//TODO: It's an axis! - kind of - for gamepads - or should this be a pov?
			case kIOHIDElementTypeInput_Axis:
				std::map<IOHIDElementCookie, AxisInfo>::iterator axisIt = std::find_if(mCookies.axisCookies.begin(), mCookies.axisCookies.end(), FindAxisCookie(event.elementCookie));
				int axis = (int)std::distance(mCookies.axisCookies.begin(), axisIt);
				
				const AxisInfo& axisInfo = axisIt->second;
				float proportion = (float) (event.value - axisInfo.max) / (float) (axisInfo.min - axisInfo.max);
				mState.mAxes[axis].abs = -MacJoyStick::MIN_AXIS - (MacJoyStick::MAX_AXIS * 2 * proportion);
				
                if(mBuffered && Gamepad::get(mDevID)->Axis != nullptr)
                    Gamepad::get(mDevID)->Axis(axis, mState.mAxes[axis].abs);
				break;
		}
		
		result = (*mQueue)->getNextEvent(mQueue, &event, zeroTime, 0);
	}
}

//--------------------------------------------------------------------------------------------------//
void MacJoyStick::setBuffered(bool buffered)
{
	mBuffered = buffered;
}

//--------------------------------------------------------------------------------------------------//
void MacJoyStick::_enumerateCookies()
{
	assert(mInfo != 0);
	assert(mInfo->interface != 0);
	
	CFTypeRef                               object; 
	long                                    number; 
	IOHIDElementCookie                      cookie; 
	long                                    usage; 
	long                                    usagePage;
	int										min;
	int										max;

	CFDictionaryRef                         element; 
	
	// Copy all elements, since we're grabbing most of the elements 
	// for this device anyway, and thus, it's faster to iterate them 
	// ourselves. When grabbing only one or two elements, a matching 
	// dictionary should be passed in here instead of NULL. 
	CFArrayRef elements; 
	IOReturn success = reinterpret_cast<IOHIDDeviceInterface122*>(*mInfo->interface)->copyMatchingElements(mInfo->interface, NULL, &elements); 
	
	if (success == kIOReturnSuccess)
	{ 
		const CFIndex numOfElements = CFArrayGetCount(elements);
		for (CFIndex i = 0; i < numOfElements; ++i) 
		{ 
			element = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(elements, i));
			
			//Get cookie 
			object = (CFDictionaryGetValue(element, 
										   CFSTR(kIOHIDElementCookieKey))); 
			if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) 
				continue; 
			if(!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, 
								 &number)) 
				continue; 
			cookie = (IOHIDElementCookie) number; 
			
			//Get usage 
			object = CFDictionaryGetValue(element, 
										  CFSTR(kIOHIDElementUsageKey)); 
			if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) 
				continue; 
			if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, 
								  &number)) 
				continue; 
			usage = number; 
			
			//Get min
			object = CFDictionaryGetValue(element,
										  CFSTR(kIOHIDElementMinKey)); // kIOHIDElementMinKey or kIOHIDElementScaledMinKey?, no idea ...
			if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
				continue;
			if (!CFNumberGetValue((CFNumberRef) object, kCFNumberIntType,
								  &number))
				continue;
			min = (int)number;
			
			//Get max
			object = CFDictionaryGetValue(element,
										  CFSTR(kIOHIDElementMaxKey)); // kIOHIDElementMaxKey or kIOHIDElementScaledMaxKey?, no idea ...
			if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
				continue;
			if (!CFNumberGetValue((CFNumberRef) object, kCFNumberIntType,
								  &number))
				continue;
			max = (int)number;
			
			//Get usage page 
			object = CFDictionaryGetValue(element, 
										  CFSTR(kIOHIDElementUsagePageKey)); 
			
			if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) 
				continue; 
			
			if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, 
								  &number)) 
				continue; 
			
			usagePage = number;
			switch(usagePage)
			{
				case kHIDPage_GenericDesktop:
					switch(usage)
				{
					case kHIDUsage_GD_Pointer:
						break;
					case kHIDUsage_GD_X:
					case kHIDUsage_GD_Y:
					case kHIDUsage_GD_Z:
					case kHIDUsage_GD_Rx:
					case kHIDUsage_GD_Ry:
					case kHIDUsage_GD_Rz:
						mCookies.axisCookies.insert(std::make_pair(cookie, AxisInfo(min, max)));
						break;
					case kHIDUsage_GD_Slider:
					case kHIDUsage_GD_Dial:
					case kHIDUsage_GD_Wheel:
						break;
					case kHIDUsage_GD_Hatswitch:
						break;
				}
					break;
				case kHIDPage_Button:
					mCookies.buttonCookies.push_back(cookie);
					break;
			}		
		}
		
		mInfo->numButtons = (int)mCookies.buttonCookies.size();
		mInfo->numAxes = (int)mCookies.axisCookies.size();

	} else {
        Kore::log(Kore::LogLevel::Info, "JoyStick elements could not be copied: copyMatchingElements failed with error: %d", success);
	}
	
}

//--------------------------------------------------------------------------------------------------//
IOHIDQueueInterface** MacJoyStick::_createQueue(unsigned int depth)
{	
	assert(mInfo != 0);
	assert(mInfo->interface != 0);
	
	IOHIDQueueInterface** queue = (*mInfo->interface)->allocQueue(mInfo->interface); 
	
	if (queue)  {
		//create the queue 
		IOReturn result = (*queue)->create(queue, 0, depth); 
		
		if(result == kIOReturnSuccess) {
			//add elements to the queue
			std::map<IOHIDElementCookie, AxisInfo>::iterator axisIt = mCookies.axisCookies.begin();
			for(; axisIt != mCookies.axisCookies.end(); ++axisIt) {
				result = (*queue)->addElement(queue, axisIt->first, 0);
			}
			
			std::vector<IOHIDElementCookie>::iterator buttonIt = mCookies.buttonCookies.begin();
			for(; buttonIt != mCookies.buttonCookies.end(); ++buttonIt) {
				result = (*queue)->addElement(queue, (*buttonIt), 0);
			}

			//start data delivery to queue 
			result = (*queue)->start(queue); 
			if(result == kIOReturnSuccess) {
				return queue;
			} else {
				Kore::log(Kore::LogLevel::Info, "Queue could not be started.");
			}
		} else {
				Kore::log(Kore::LogLevel::Info, "Queue could not be created.");
		}
	} else {
        Kore::log(Kore::LogLevel::Info, "Queue allocation failed.");
	}
    
    return 0;
}