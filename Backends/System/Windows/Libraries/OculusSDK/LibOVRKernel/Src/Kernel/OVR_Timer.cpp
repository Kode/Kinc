/************************************************************************************

Filename    :   OVR_Timer.cpp
Content     :   Provides static functions for precise timing
Created     :   September 19, 2012
Notes       :

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

#include "OVR_Timer.h"
#include "OVR_Log.h"

#if defined(OVR_OS_MS) && !defined(OVR_OS_MS_MOBILE)
#include "OVR_Win32_IncludeWindows.h"
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#elif defined(OVR_OS_ANDROID)
#include <time.h>
#include <android/log.h>
#else
#include <chrono>
#endif

#if defined(OVR_BUILD_DEBUG) && defined(OVR_OS_WIN32)
typedef NTSTATUS(
    NTAPI* NtQueryTimerResolutionType)(PULONG MaximumTime, PULONG MinimumTime, PULONG CurrentTime);
static NtQueryTimerResolutionType pNtQueryTimerResolution;
#endif

#if defined(OVR_OS_MS) && !defined(OVR_OS_WIN32) // Non-desktop Microsoft platforms...

// Add this alias here because we're not going to include OVR_CAPI.cpp
extern "C" {
double ovr_GetTimeInSeconds() {
  return Timer::GetSeconds();
}
}

#endif

namespace OVR {

// For recorded data playback
bool Timer::useVirtualSeconds = false;
double Timer::VirtualSeconds = 0.0;

//------------------------------------------------------------------------
// *** Android Specific Timer

#if defined( \
    OVR_OS_ANDROID) // To consider: This implementation can also work on most Linux distributions

//------------------------------------------------------------------------
// *** Timer - Platform Independent functions

// Returns global high-resolution application timer in seconds.
double Timer::GetSeconds() {
  if (useVirtualSeconds)
    return VirtualSeconds;

  // Choreographer vsync timestamp is based on.
  struct timespec tp;
  const int status = clock_gettime(CLOCK_MONOTONIC, &tp);

#ifdef OVR_BUILD_DEBUG
  if (status != 0) {
    OVR_DEBUG_LOG(("clock_gettime status=%i", status));
  }
#else
  OVR_UNUSED(status);
#endif

  return (double)tp.tv_sec;
}

uint64_t Timer::GetTicksNanos() {
  if (useVirtualSeconds)
    return (uint64_t)(VirtualSeconds * NanosPerSecond);

  // Choreographer vsync timestamp is based on.
  struct timespec tp;
  const int status = clock_gettime(CLOCK_MONOTONIC, &tp);

#ifdef OVR_BUILD_DEBUG
  if (status != 0) {
    OVR_DEBUG_LOG(("clock_gettime status=%i", status));
  }
#else
  OVR_UNUSED(status);
#endif

  const uint64_t result =
      (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
  return result;
}

void Timer::initializeTimerSystem() {
  // Empty for this platform.
}

void Timer::shutdownTimerSystem() {
  // Empty for this platform.
}

//------------------------------------------------------------------------
// *** Win32 Specific Timer

#elif defined(OVR_OS_MS)

// This helper class implements high-resolution wrapper that combines timeGetTime() output
// with QueryPerformanceCounter.  timeGetTime() is lower precision but drives the high bits,
// as it's tied to the system clock.
struct PerformanceTimer {
  PerformanceTimer()
      : UsingVistaOrLater(false),
        TimeCS(),
        OldMMTimeMs(0),
        MMTimeWrapCounter(0),
        PerfFrequency(0),
        PerfFrequencyInverse(0),
        PerfFrequencyInverseNanos(0),
        PerfMinusTicksDeltaNanos(0),
        LastResultNanos(0) {}

  enum { MMTimerResolutionNanos = 1000000 };

  void Initialize();
  void Shutdown();

  uint64_t GetTimeSeconds();
  double GetTimeSecondsDouble();
  uint64_t GetTimeNanos();

  UINT64 getFrequency() {
    if (PerfFrequency == 0) {
      LARGE_INTEGER freq;
      ::QueryPerformanceFrequency(&freq);
      PerfFrequency = freq.QuadPart;
      PerfFrequencyInverse = 1.0 / (double)PerfFrequency;
      PerfFrequencyInverseNanos = 1000000000.0 / (double)PerfFrequency;
    }
    return PerfFrequency;
  }

  double GetFrequencyInverse() {
    OVR_ASSERT(PerfFrequencyInverse != 0.0); // Assert that the frequency has been initialized.
    return PerfFrequencyInverse;
  }

  // In Vista+ we are able to use QPC exclusively.
  bool UsingVistaOrLater;

  CRITICAL_SECTION TimeCS;
  // timeGetTime() support with wrap.
  uint32_t OldMMTimeMs;
  uint32_t MMTimeWrapCounter;
  // Cached performance frequency result.
  uint64_t PerfFrequency; // cycles per second, typically a large value like 3000000, but usually
  // not the same as the CPU clock rate.
  double PerfFrequencyInverse; // seconds per cycle (will be a small fractional value).
  double PerfFrequencyInverseNanos; // nanoseconds per cycle.

  // Computed as (perfCounterNanos - ticksCounterNanos) initially,
  // and used to adjust timing.
  uint64_t PerfMinusTicksDeltaNanos;
  // Last returned value in nanoseconds, to ensure we don't back-step in time.
  uint64_t LastResultNanos;
};

static PerformanceTimer Win32_PerfTimer;

void PerformanceTimer::Initialize() {
  ::InitializeCriticalSection(&TimeCS);
  MMTimeWrapCounter = 0;
  getFrequency();

#if defined(OVR_OS_WIN32) // Desktop Windows only
  // Set Vista flag.  On Vista, we can just use QPC() without all the extra work
  OSVERSIONINFOEXW ver;
  ZeroMemory(&ver, sizeof(OSVERSIONINFOEXW));
  ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
  ver.dwMajorVersion = 6; // Vista+

  DWORDLONG condMask = 0;
  VER_SET_CONDITION(condMask, VER_MAJORVERSION, VER_GREATER_EQUAL);

  // VerifyVersionInfo returns true if the OS meets the conditions set above
  UsingVistaOrLater = ::VerifyVersionInfoW(&ver, VER_MAJORVERSION, condMask) != 0;
#else
  UsingVistaOrLater = true;
#endif

  if (!UsingVistaOrLater) {
#if defined(OVR_OS_WIN32) // Desktop Windows only
    // The following has the effect of setting the NT timer resolution (NtSetTimerResolution) to 1
    // millisecond.
    MMRESULT mmr = timeBeginPeriod(1);
    OVR_ASSERT(TIMERR_NOERROR == mmr);
    OVR_UNUSED(mmr);
#endif

#if defined(OVR_BUILD_DEBUG) && defined(OVR_OS_WIN32)
    HMODULE hNtDll = ::LoadLibraryW(L"NtDll.dll");
    if (hNtDll) {
      pNtQueryTimerResolution =
          (NtQueryTimerResolutionType)::GetProcAddress(hNtDll, "NtQueryTimerResolution");
      // pNtSetTimerResolution = (NtSetTimerResolutionType)::GetProcAddress(hNtDll,
      // "NtSetTimerResolution");

      if (pNtQueryTimerResolution) {
        ULONG MinimumResolution; // in 100-ns units
        ULONG MaximumResolution;
        ULONG ActualResolution;
        pNtQueryTimerResolution(&MinimumResolution, &MaximumResolution, &ActualResolution);
        OVR_DEBUG_LOG(
            ("NtQueryTimerResolution = Min %ld us, Max %ld us, Current %ld us",
             MinimumResolution / 10,
             MaximumResolution / 10,
             ActualResolution / 10));
      }

      ::FreeLibrary(hNtDll);
    }
#endif
  }
}

void PerformanceTimer::Shutdown() {
  ::DeleteCriticalSection(&TimeCS);

  if (!UsingVistaOrLater) {
#if defined(OVR_OS_WIN32) // Desktop Windows only
    MMRESULT mmr = timeEndPeriod(1);
    OVR_ASSERT(TIMERR_NOERROR == mmr);
    OVR_UNUSED(mmr);
#endif
  }
}

uint64_t PerformanceTimer::GetTimeSeconds() {
  if (UsingVistaOrLater) {
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    OVR_ASSERT(PerfFrequencyInverse != 0); // Initialize should have been called earlier.
    return (uint64_t)(li.QuadPart * PerfFrequencyInverse);
  }

  return (uint64_t)(GetTimeNanos() * .0000000001);
}

double PerformanceTimer::GetTimeSecondsDouble() {
  if (UsingVistaOrLater) {
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    OVR_ASSERT(PerfFrequencyInverse != 0);
    return (li.QuadPart * PerfFrequencyInverse);
  }

  return (GetTimeNanos() * .0000000001);
}

uint64_t PerformanceTimer::GetTimeNanos() {
  uint64_t resultNanos;
  LARGE_INTEGER li;

  OVR_ASSERT(PerfFrequencyInverseNanos != 0); // Initialize should have been called earlier.

  if (UsingVistaOrLater) // Includes non-desktop platforms
  {
    // Then we can use QPC() directly without all that extra work
    ::QueryPerformanceCounter(&li);
    resultNanos = (uint64_t)(li.QuadPart * PerfFrequencyInverseNanos);
    return resultNanos;
  }

  // Pre-Vista computers:
  // Note that the Oculus SDK does not run on PCs before Windows 7 SP1
  // so this code path should never be taken in practice.  We keep it here
  // since this is a nice reusable timing library that can be useful for
  // other projects.

  // On Win32 QueryPerformanceFrequency is unreliable due to SMP and
  // performance levels, so use this logic to detect wrapping and track
  // high bits.
  ::EnterCriticalSection(&TimeCS);

  // Get raw value and perf counter "At the same time".
  ::QueryPerformanceCounter(&li);

  DWORD mmTimeMs = timeGetTime();
  if (OldMMTimeMs > mmTimeMs)
    MMTimeWrapCounter++;
  OldMMTimeMs = mmTimeMs;

  // Normalize to nanoseconds.
  uint64_t perfCounterNanos = (uint64_t)(li.QuadPart * PerfFrequencyInverseNanos);
  uint64_t mmCounterNanos = ((uint64_t(MMTimeWrapCounter) << 32) | mmTimeMs) * 1000000;
  if (PerfMinusTicksDeltaNanos == 0)
    PerfMinusTicksDeltaNanos = perfCounterNanos - mmCounterNanos;

  // Compute result before snapping.
  //
  // On first call, this evaluates to:
  //          resultNanos = mmCounterNanos.
  // Next call, assuming no wrap:
  //          resultNanos = prev_mmCounterNanos + (perfCounterNanos - prev_perfCounterNanos).
  // After wrap, this would be:
  //          resultNanos = snapped(prev_mmCounterNanos +/- 1ms) + (perfCounterNanos -
  //          prev_perfCounterNanos).
  //
  resultNanos = perfCounterNanos - PerfMinusTicksDeltaNanos;

  // Snap the range so that resultNanos never moves further apart then its target resolution.
  // It's better to allow more slack on the high side as timeGetTime() may be updated at
  // sporadically
  // larger then 1 ms intervals even when 1 ms resolution is requested.
  if (resultNanos > (mmCounterNanos + MMTimerResolutionNanos * 2)) {
    resultNanos = mmCounterNanos + MMTimerResolutionNanos * 2;
    if (resultNanos < LastResultNanos)
      resultNanos = LastResultNanos;
    PerfMinusTicksDeltaNanos = perfCounterNanos - resultNanos;
  } else if (resultNanos < (mmCounterNanos - MMTimerResolutionNanos)) {
    resultNanos = mmCounterNanos - MMTimerResolutionNanos;
    if (resultNanos < LastResultNanos)
      resultNanos = LastResultNanos;
    PerfMinusTicksDeltaNanos = perfCounterNanos - resultNanos;
  }

  LastResultNanos = resultNanos;
  ::LeaveCriticalSection(&TimeCS);

  return resultNanos;
}

//------------------------------------------------------------------------
// *** Timer - Platform Independent functions

// Returns global high-resolution application timer in seconds.
double Timer::GetSeconds() {
  return Win32_PerfTimer.GetTimeSecondsDouble();
}

double Timer::GetVirtualSeconds() {
  if (useVirtualSeconds)
    return VirtualSeconds;

  return Win32_PerfTimer.GetTimeSecondsDouble();
}

// Delegate to PerformanceTimer.
uint64_t Timer::GetVirtualTicksNanos() {
  if (useVirtualSeconds)
    return (uint64_t)(VirtualSeconds * NanosPerSecond);

  return Win32_PerfTimer.GetTimeNanos();
}

uint64_t Timer::GetTicksNanos() {
  return Win32_PerfTimer.GetTimeNanos();
}

// Windows version also provides the performance frequency inverse.
double Timer::GetPerfFrequencyInverse() {
  return Win32_PerfTimer.GetFrequencyInverse();
}

void Timer::initializeTimerSystem() {
  Win32_PerfTimer.Initialize();
}
void Timer::shutdownTimerSystem() {
  Win32_PerfTimer.Shutdown();
}

#else // C++11 standard compliant platforms

double Timer::GetSeconds() {
  if (useVirtualSeconds)
    return VirtualSeconds;

  using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;

  auto now = std::chrono::high_resolution_clock::now();
  return FpSeconds(now.time_since_epoch()).count();
}

double Timer::GetVirtualSeconds() {
  if (useVirtualSeconds)
    return VirtualSeconds;

  using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;

  auto now = std::chrono::high_resolution_clock::now();
  return FpSeconds(now.time_since_epoch()).count();
}

uint64_t Timer::GetTicksNanos() {
  if (useVirtualSeconds)
    return (uint64_t)(VirtualSeconds * NanosPerSecond);

  using Uint64Nanoseconds = std::chrono::duration<uint64_t, std::chrono::nanoseconds::period>;

  auto now = std::chrono::high_resolution_clock::now();
  return Uint64Nanoseconds(now.time_since_epoch()).count();
}

void Timer::initializeTimerSystem() {}

void Timer::shutdownTimerSystem() {}

#endif // OS-specific

CountdownTimer::CountdownTimer(size_t countdownTimeMs, bool start)
    : CountdownTime(std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          std::chrono::milliseconds(countdownTimeMs))) {
  if (start)
    Restart();
}

std::chrono::steady_clock::time_point CountdownTimer::CurrentTime() const {
  return std::chrono::steady_clock::now();
}

bool CountdownTimer::IsTimeUp() const {
  return (CurrentTime() > DoneTime);
}

void CountdownTimer::Restart() {
  DoneTime = (CurrentTime() + CountdownTime);
}

void CountdownTimer::Restart(size_t countdownTimeMs) {
  CountdownTime = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::milliseconds(countdownTimeMs));
  Restart();
};

} // namespace OVR
