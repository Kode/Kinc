#pragma once

#include <Kore/Gamepad/MacHIDManager.h>
#include <Kore/Math/Vector.h>

namespace Kore
{
    //! Axis component
    class Axis
    {
    public:
        Axis() : abs(0), rel(0), absOnly(false) {};
        
        //! Absoulte and Relative value components
        int abs, rel;
        
        //! Indicates if this Axis only supports Absoulte (ie JoyStick)
        bool absOnly;
        
        //! Used internally by OIS
        void clear()
        {
            abs = rel = 0;
        }
    };
    
    //! POV / HAT Joystick component
    class Pov
    {
    public:
        Pov() : direction(0) {}
        
        static const int Centered  = 0x00000000;
        static const int North     = 0x00000001;
        static const int South     = 0x00000010;
        static const int East      = 0x00000100;
        static const int West      = 0x00001000;
        static const int NorthEast = 0x00000101;
        static const int SouthEast = 0x00000110;
        static const int NorthWest = 0x00001001;
        static const int SouthWest = 0x00001010;
        
        int direction;
    };
    
    //! A sliding axis - only used in Win32 Right Now
    class Slider
    {
    public:
        Slider() : abX(0), abY(0) {};
        //! true if pushed, false otherwise
        int abX, abY;
    };
    
	struct AxisInfo
	{
		int min;
		int max;
		
		AxisInfo(int min, int max)
			: min(min), max(max) {}
	};
	
	typedef struct cookie_struct 
	{ 
		std::map<IOHIDElementCookie, AxisInfo> axisCookies; 			
		std::vector<IOHIDElementCookie> buttonCookies; 
	} cookie_struct_t;
	
	/**
		Represents the state of the joystick
		All members are valid for both buffered and non buffered mode
		Sticks with zero values are not present on the device
	 */
	class JoyStickState
	{
	public:
		//! Constructor
		JoyStickState() { clear(); }
		
		//! Represents all the buttons (uses a bitset)
		std::vector<bool> mButtons;
		
		//! Represents all the single axes on the device
		std::vector<Axis> mAxes;
		
		//! Represents the value of a POV. Maximum of 4
		Pov mPOV[4];
		
		//! Represent the max sliders
		Slider mSliders[4];
		
		//! Represents all Vector type controls the device exports
		std::vector<vec3> mVectors;
		
		//! internal method to reset all variables to initial values
		void clear()
		{
			for( std::vector<bool>::iterator i = mButtons.begin(), e = mButtons.end(); i != e; ++i )
			{
				(*i) = false;
			}
			
			for( std::vector<Axis>::iterator i = mAxes.begin(), e = mAxes.end(); i != e; ++i )
			{
				i->absOnly = true; //Currently, joysticks only report Absolute values
				i->clear();
			}
			
			for( std::vector<vec3>::iterator i = mVectors.begin(), e = mVectors.end(); i != e; ++i )
			{
				i->set(0, 0, 0);
			}
			
			for( int i = 0; i < 4; ++i )
			{
				mPOV[i].direction = Pov::Centered;
				mSliders[i].abX = mSliders[i].abY = 0;
			}
		}
	};
	
	//class HidDeviceInfo
	
	class MacJoyStick
	{
	public:
		MacJoyStick(const std::string& vendor, bool buffered, HidInfo* info, int devID);
		
		virtual ~MacJoyStick();
		
		/** @copydoc Object::setBuffered */
		virtual void setBuffered(bool buffered);
		
		/** @copydoc Object::capture */
		virtual void capture();
				
		/** @copydoc Object::_initialize */
		virtual void _initialize();
		
		void _enumerateCookies();
        
        //! Vendor name if applicable/known
        std::string* mVendor;
        
		IOHIDQueueInterface** _createQueue(unsigned int depth = 8);
	protected:
		HidInfo* mInfo;
		cookie_struct_t mCookies;
		IOHIDQueueInterface** mQueue;
		
        //! Buffered flag
        bool mBuffered;
        
        //! Not fully implemented yet
        int mDevID;
        
		//! Number of sliders
		int mSliders;
		
		//! Number of POVs
		int mPOVs;
		
		//! The minimal axis value
		static const int MIN_AXIS = -32768;
		
		//! The maximum axis value
		static const int MAX_AXIS = 32767;
		
		//! The JoyStickState structure (contains all component values)
		JoyStickState mState;
	};
}
