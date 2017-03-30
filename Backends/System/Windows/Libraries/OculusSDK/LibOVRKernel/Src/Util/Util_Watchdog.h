/************************************************************************************

Filename    :   Util_Watchdog.h
Content     :   Deadlock reaction
Created     :   June 27, 2013
Authors     :   Kevin Jenkins, Chris Taylor

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

*************************************************************************************/

#ifndef OVR_Util_Watchdog_h
#define OVR_Util_Watchdog_h

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_Allocator.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Threads.h"
#include "Kernel/../../../Logging/include/Logging_Library.h"

namespace OVR { namespace Util {


//-----------------------------------------------------------------------------
// WatchDog

class WatchDog : public NewOverrideBase
{
    friend class WatchDogObserver;

public:
    WatchDog(const String& threadName);
    ~WatchDog();

    void Disable();
    void Enable();

    void Feed(int threshold);

protected:
    // Use 32 bit int so assignment and comparison is atomic
    std::atomic<uint32_t> WhenLastFedMilliseconds = { 0 };
    std::atomic<int>      ThreshholdMilliseconds = { 0 };

    String              ThreadName;
    bool                Listed;
};


//-----------------------------------------------------------------------------
// WatchDogObserver

class WatchDogObserver : public SystemSingletonBase<WatchDogObserver>
{
    OVR_DECLARE_SINGLETON(WatchDogObserver);
    virtual void OnThreadDestroy() override;

    friend class WatchDog;

    std::unique_ptr<std::thread> WatchdogThreadHandle;

public:
    // Uses the exception logger to write deadlock reports
    void EnableReporting(const String organization = String(),
                         const String application = String());

    // Disables deadlock reports
    void DisableReporting();

    // This is the delay between deciding a deadlock occurred and terminating the process, to allow for logging
    static const int TerminationDelayMsec = 10000; // 10 seconds in msec

    // Enable/disable auto-terminate on deadlock report
    // IsDeadlocked() will return true, and after TerminationDelayMsec it will exit the process
    void SetAutoTerminateOnDeadlock(bool enable = true)
    {
        AutoTerminateOnDeadlock = enable;
    }

    // Is currently in a deadlock state?
    bool IsDeadlocked() const
    {
        return DeadlockSeen;
    }

protected:
    Lock ListLock;
    Array< WatchDog* > DogList;

    // This indicates that EnableReporting() was requested
    bool IsReporting = false;

    Event TerminationEvent;

    // Has a deadlock been seen?
    bool DeadlockSeen = false;
    bool AutoTerminateOnDeadlock = true;

    // On Windows, deadlock logs are stored in %AppData%\OrganizationName\ApplicationName\.
    // See ExceptionHandler::ReportDeadlock() for how these are used.
    String ApplicationName;
    String OrganizationName;

    void OnDeadlock(const String& deadlockedThreadName);

protected:
    int Run();

    void Add(WatchDog* dog);
    void Remove(WatchDog* dog);
};


}} // namespace OVR::Util

#endif // OVR_Util_Watchdog_h
