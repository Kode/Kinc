/************************************************************************************

Filename    :   OVR_Log.h
Content     :   Logging support
Created     :   September 19, 2012

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License"); 
you may not use the Oculus VR Rift SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Log_h
#define OVR_Log_h

#ifdef MICRO_OVR

namespace OVR {
template<typename... Args> void LogText(Args&&...) {}
template<typename... Args> void LogError(Args&&...) {}
template<typename... Args> void LogDebug(Args&&...) {}
}

#define OVR_DEBUG_LOG(args)       do {} while(0);
#define OVR_DEBUG_LOG_TEXT(args)  do {} while(0);
#define OVR_ASSERT_LOG(c, args)   do {} while(0)

namespace ovrlog {
    struct Channel{
        template<typename... Args>Channel(Args&&...){}
        template<typename... Args> void LogTrace(Args&&...){}
        template<typename... Args> void LogWarningF(Args&&...){}
    };
}

#else

#include "OVR_Types.h"
#include "OVR_Std.h"
#include "OVR_String.h"

#include "../../../Logging/include/Logging_Library.h"

namespace OVR {


//-----------------------------------------------------------------------------------
// Terrible Legacy Log Wrapper
//
// Try not to use this deprecated code.

#define OVR_SYSLOG_NAME L"OculusVR"

typedef int LogMessageType;

extern ovrlog::Channel DefaultChannel;

template<typename... Args>
inline void LogText(Args&&... args)
{
    if (DefaultChannel.Active(ovrlog::Level::Info))
    {
        char buffer[512];
        int written = snprintf(buffer, sizeof(buffer), args...);
        if (written <= 0 || written >= (int)sizeof(buffer))
        {
            OVR_ASSERT(false); // This call should be converted to the new log system.
            return;
        }

        // Fix up the newlines
        if (buffer[written - 1] == '\n')
        {
            buffer[written - 1] = '\0';
        }

        DefaultChannel.LogInfo(buffer);
    }
}

template<typename... Args>
inline void LogError(Args&&... args)
{
    if (DefaultChannel.Active(ovrlog::Level::Error))
    {
        char buffer[512];
        int written = snprintf(buffer, sizeof(buffer), args...);
        if (written <= 0 || written >= (int)sizeof(buffer))
        {
            OVR_ASSERT(false); // This call should be converted to the new log system.
            return;
        }

        // Fix up the newlines
        if (buffer[written - 1] == '\n')
        {
            buffer[written - 1] = '\0';
        }

        DefaultChannel.LogError(buffer);
    }
}

template<typename... Args>
inline void LogDebug(Args&&... args)
{
    if (DefaultChannel.Active(ovrlog::Level::Debug))
    {
        char buffer[512];
        int written = snprintf(buffer, sizeof(buffer), args...);
        if (written <= 0 || written >= (int)sizeof(buffer))
        {
            OVR_ASSERT(false); // This call should be converted to the new log system.
            return;
        }

        // Fix up the newlines
        if (buffer[written - 1] == '\n')
        {
            buffer[written - 1] = '\0';
        }

        DefaultChannel.LogDebug(buffer);
    }
}

#define OVR_DEBUG_LOG(args)      do { OVR::LogDebug args; } while(0);
#define OVR_DEBUG_LOG_TEXT(args) do { OVR::LogDebug args; } while(0);
#define OVR_ERROR_LOG(args)      do { OVR::LogError args; } while(0);

// Conditional logging & asserting. It asserts/logs when the condition 'c' is NOT true.
#define OVR_ASSERT_LOG(c, args)      do { if (!(c)) { OVR::LogError args; OVR_DEBUG_BREAK; } } while(0)


} // namespace OVR 


namespace ovrlog {

    template<> LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const OVR::String& first)
    {
        buffer.Stream << first.ToCStr();
    }

} // namespace ovrlog

#endif // MICRO_OVR

#endif // OVR_Log_h
