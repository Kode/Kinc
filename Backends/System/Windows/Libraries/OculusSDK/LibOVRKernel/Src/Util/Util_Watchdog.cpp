/************************************************************************************

Filename    :   Util_Watchdog.cpp
Content     :   Deadlock reaction
Created     :   June 27, 2013
Authors     :   Kevin Jenkins, Chris Taylor

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

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

#include "Util_Watchdog.h"

#include <Logging/Logging_Library.h>

#include "Kernel/OVR_DebugHelp.h"
#include "Kernel/OVR_Win32_IncludeWindows.h"

#if defined(OVR_OS_LINUX) || defined(OVR_OS_MAC)
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#if defined(OVR_OS_LINUX)
#include <sys/wait.h>
#endif
#endif

OVR_DEFINE_SINGLETON(OVR::Util::WatchDogObserver);

namespace OVR {
namespace Util {

// Watchdog class default threshold before announcing a long cycle
const int DefaultThreshholdMsec = 60000; // milliseconds

//-----------------------------------------------------------------------------
// Tools

static uint32_t GetFastMsTime() {
#if defined(OVR_OS_MS)
  return ::GetTickCount();
#else
  return Timer::GetTicksMs();
#endif
}

static std::string SanitizeString(const char* cstr) {
  std::ostringstream ss;
  char ch;
  while (ch = *cstr++, ch != '\0') {
    if (ch < ' ' || ch == '\"' || ch == '\'' || ch == '\\' || ch == '`' || ch > '~')
      ch = '_';
    ss << ch;
  }
  return ss.str();
}

//-----------------------------------------------------------------------------
// WatchDogObserver

static ovrlog::Channel Logger("Kernel:Watchdog");

WatchDogObserver::WatchDogObserver()
    : ListLock(),
      DogList(),
      IsReporting(false),
      TerminationEvent(),
      DeadlockSeen(false),
      AutoTerminateOnDeadlock(true),
      ApplicationName(),
      OrganizationName(),
      WriteMiniDump(nullptr),
      AddBreakpadInfoClient(nullptr) {
  WatchdogThreadHandle = std::make_unique<std::thread>([this] { this->Run(); });
  // Must be at end of function
  PushDestroyCallbacks();
}

WatchDogObserver::~WatchDogObserver() {
  OVR_ASSERT(!WatchdogThreadHandle->joinable());
}

void WatchDogObserver::OnThreadDestroy() {
  Logger.LogDebug("Setting TerminationEvent for watchdog thread.");
  TerminationEvent.SetEvent();

  WatchdogThreadHandle->join();
}

void WatchDogObserver::OnSystemDestroy() {
  // NOTE: Explicitly allowed to do this by the top-level comment of OnSystemDestroy()
  delete this;
}

void WatchDogObserver::OnDeadlock(const String& deadlockedThreadName) {
  // If is reporting deadlock details:
  if (IsReporting) {
    if (SymbolLookup::Initialize()) {
      // Static to avoid putting 32 KB on the stack.  This is only called once, so it's safe.
      static SymbolLookup symbolLookup;
      String threadListOutput, moduleListOutput;
      symbolLookup.ReportThreadCallstacks(threadListOutput);
      symbolLookup.ReportModuleInformation(moduleListOutput);

      Logger.LogWarning(
          "---DEADLOCK STATE---\n\n",
          threadListOutput.c_str(),
          "\n\n",
          moduleListOutput.c_str(),
          "\n---END OF DEADLOCK STATE---");
    }

    // For internal builds done by our engineers, we write minidumps as per the settings below. We
    // don't generate breakpad minidumps nor upload breakpad crash reports. For non-internal builds
    // (typically public builds), we generate breakpad mindumps and will upload them as necessary.
    if (WriteMiniDump != nullptr) {
      // non-internal builds
      OVR_ASSERT(AddBreakpadInfoClient != nullptr);
      AddBreakpadInfoClient("is_deadlock_minidump", "true");
      // deadlockedThreadName might contain special characters
      std::string threadName = SanitizeString(deadlockedThreadName.c_str());
      AddBreakpadInfoClient("deadlock_thread_name", threadName.c_str());
      WriteMiniDump(nullptr);
    } else {
      // internal builds
      ExceptionHandler::ReportDeadlock(deadlockedThreadName, OrganizationName, ApplicationName);
    }

    // Disable reporting after the first deadlock report
    DisableReporting();
  }

  DeadlockSeen = true;

  Logger.LogError(
      "WatchDogObserver::OnDeadlock: Deadlock detected in thread '",
      deadlockedThreadName.c_str(),
      "'");

  if (OVRIsDebuggerPresent()) {
    Logger.LogWarning(
        "WatchDogObserver::OnDeadlock: Aborting termination since debugger is present. Normally the app would terminate itself here");
  } else if (AutoTerminateOnDeadlock) {
    Logger.LogError(
        "WatchDogObserver::OnDeadlock: Waiting ",
        TerminationDelayMsec,
        " msec until deadlock termination");

    if (!TerminationEvent.Wait(TerminationDelayMsec)) {
#if defined(_WIN32) // To do: Implement a generic OVR library terminate self API. With a clean
      // option (exit()) and drastic option (TerminateProcess()).
      ::TerminateProcess(GetCurrentProcess(), 0xd00ddead);
#else
      exit(0xd00ddead);
#endif
    }

    Logger.LogError(
        "WatchDogObserver::OnDeadlock: Deadlock termination aborted - Graceful shutdown");
  }
}

int WatchDogObserver::Run() {
  Thread::SetCurrentThreadName("WatchDog");

  Logger.LogDebug("WatchDogObserver::Run: Starting watchdog thread");

  // Milliseconds between checks
  static const int kWakeupIntervalMsec = 4000; // 4 seconds

  // Number of consecutive long cycles before the watchdog dumps a minidump
  static const int kLongCycleTimeLimitMsec = 60000; // 1 minute
  static const int kMaxConsecutiveLongCycles = kLongCycleTimeLimitMsec / kWakeupIntervalMsec;

  // By counting consecutive long cycles instead of using a timeout, we prevent false positives
  // from debugger breakpoints and sleep/resume of the OS.
  int ConsecutiveLongCycles = 0; // Number of long cycles seen in a row

  // While not requested to terminate:
  while (!TerminationEvent.Wait(kWakeupIntervalMsec)) {
    bool sawLongCycle = false;
    String deadlockedThreadName;

    {
      Lock::Locker locker(&ListLock);

      const uint32_t t1 = GetFastMsTime();

      const int count = DogList.GetSizeI();
      for (int i = 0; i < count; ++i) {
        const WatchDog* dog = DogList[i];

        const int threshold = dog->ThreshholdMilliseconds;
        const uint32_t t0 = dog->WhenLastFedMilliseconds;

        // If threshold exceeded, assume there is thread deadlock of some sort.
        int delta = static_cast<int>(t1 - t0);

        // Include an upper bound in case the computer went to sleep
        if (delta > threshold && (ConsecutiveLongCycles > 0 || delta < threshold * 5)) {
          sawLongCycle = true;

          deadlockedThreadName = dog->ThreadName;
          Logger.LogWarning(
              "WatchDogObserver::Run: Long cycle detected ",
              ConsecutiveLongCycles + 1,
              "x (max=",
              kMaxConsecutiveLongCycles,
              ") in thread '",
              deadlockedThreadName.c_str(),
              "'");
        }
      }
    }

    // If we did not see a long cycle:
    if (!sawLongCycle) {
      if (ConsecutiveLongCycles > 0) {
        // Reset log cycle count
        ConsecutiveLongCycles = 0;

        Logger.LogWarning("WatchDogObserver::Run: Recovered from long cycles");
      }

      // Reset deadlock seen flag
      DeadlockSeen = false;
    } else if (++ConsecutiveLongCycles >= kMaxConsecutiveLongCycles) {
      // Since it requires consecutive long cycles, waking up from sleep/resume will not trigger a
      // deadlock.
      OnDeadlock(deadlockedThreadName);
    }
  }

  Logger.LogDebug("WatchDogObserver::Run: Terminating watchdog thread");

  return 0;
}

void WatchDogObserver::Add(WatchDog* dog) {
  Lock::Locker locker(&ListLock);

  if (!dog->Listed) {
    Logger.LogDebugF("WatchDogObserver::Add Watchdog: %s", dog->GetThreadName().c_str());
    DogList.PushBack(dog);
    dog->Listed = true;
  } else
    Logger.LogDebugF(
        "WatchDogObserver::Add Watchdog: %s already added.", dog->GetThreadName().c_str());
}

void WatchDogObserver::Remove(WatchDog* dog) {
  Lock::Locker locker(&ListLock);
  bool removed = false;

  Logger.LogDebugF("WatchDogObserver::Remove Watchdog: %s", dog->GetThreadName().c_str());

  if (dog->Listed) {
    for (int i = 0; i < DogList.GetSizeI(); ++i) {
      if (DogList[i] == dog) {
        DogList.RemoveAt(i--);
        removed = true;
        break;
      }
    }

    dog->Listed = false;
  }

  if (!removed)
    Logger.LogDebugF(
        "WatchDogObserver::Remove Watchdog: %s already removed.", dog->GetThreadName().c_str());
}

void WatchDogObserver::EnableReporting(const String& organization, const String& application) {
  OrganizationName = organization;
  ApplicationName = application;
  IsReporting = true;
}

void WatchDogObserver::DisableReporting() {
  IsReporting = false;
}

//-----------------------------------------------------------------------------
// WatchDog

WatchDog::WatchDog(const String& threadName)
    : ThreshholdMilliseconds(DefaultThreshholdMsec), ThreadName(threadName), Listed(false) {
  WhenLastFedMilliseconds = GetFastMsTime();

  OVR_ASSERT(!ThreadName.empty());
}

WatchDog::~WatchDog() {
  Disable();
}

String WatchDog::GetThreadName() const {
  return ThreadName;
}

void WatchDog::Disable() {
  WatchDogObserver::GetInstance()->Remove(this);
}

void WatchDog::Enable() {
  WatchDogObserver::GetInstance()->Add(this);
}

void WatchDog::Feed(int threshold) {
  WhenLastFedMilliseconds = GetFastMsTime();
  ThreshholdMilliseconds = threshold;

  if (!Listed) {
    Enable();
  }
}
} // namespace Util
} // namespace OVR
