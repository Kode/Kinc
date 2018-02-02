/************************************************************************************

Filename    :   Logging_Tools.cpp
Content     :   Tools for Logging
Created     :   Oct 26, 2015
Authors     :   Chris Taylor

Copyright   :   Copyright 2015-2016 Oculus VR, LLC All Rights reserved.

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

#ifdef _MSC_VER
    #pragma warning(disable: 4530) // C++ exception handler used, but unwind semantics are not enabled
#endif

#include "Logging_Tools.h"

#include <assert.h>
#include <time.h>

#include <chrono>
#include <codecvt>
#include <vector>

#if defined(_MSC_VER)
    #include <filesystem> // Pre-release C++ filesystem support.
#endif

#if defined(__APPLE__)
    #include <unistd.h>
    #include <sys/sysctl.h>
    #include <libproc.h>
#endif


namespace ovrlog {


//-----------------------------------------------------------------------------
// Terminator

Terminator::Terminator() :
    Terminated(false),
    TerminateEvent()
{
}

Terminator::~Terminator()
{
}

bool Terminator::Initialize()
{
    Terminated = false;

    #if defined(_WIN32)
        if (TerminateEvent.IsValid())
        {
            ::ResetEvent(TerminateEvent.Get());
            return true;
        }

        TerminateEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    #else
        // To do: Implement this.
        fprintf(stderr, __FILE__ "[%s] Not implemented.\n", __func__);
    #endif
    
    return TerminateEvent.IsValid();
}

void Terminator::Terminate()
{
    Terminated = true;

    if (TerminateEvent.IsValid())
    {
        #if defined(_WIN32)
            ::SetEvent(TerminateEvent.Get());
        #else
            // To do: Implement this.
            assert(false);
        #endif
    }
}

// Returns true if the event signaled and false on termination.
bool Terminator::WaitOn(OvrLogHandle hEvent, uint32_t timeoutMsec)
{
    if (Terminated || !TerminateEvent.IsValid())
        return false;

    #if defined(_WIN32)
        HANDLE events[2] = { hEvent, TerminateEvent.Get() };

        DWORD result = ::WaitForMultipleObjects(2, events, FALSE, timeoutMsec);

        if (Terminated)
            return false;

        if (result == WAIT_TIMEOUT)
            return false;
        if (result == WAIT_OBJECT_0)
            return true;
    #else
        // To do: Implement this.
        (void)hEvent;
        (void)timeoutMsec;
        assert(false);
    #endif
    
    return false;
}

// Returns true if the sleep interval exceeded or false on termination.
bool Terminator::WaitSleep(int milliseconds)
{
    if (Terminated || !TerminateEvent.IsValid())
        return false;

    #if defined(_WIN32)
        ::WaitForSingleObject(TerminateEvent.Get(), milliseconds); // Ignore return value
    #else
        // To do: Implement this.
        (void)milliseconds;
        assert(false);
    #endif

    
    return !Terminated;
}


//-----------------------------------------------------------------------------
// Lock

Lock::Lock() :
#if defined(_WIN32)
    cs{}
#else
	m()
#endif
{
#if defined(_WIN32)
    static const DWORD kSpinCount = 1000;
    ::InitializeCriticalSectionAndSpinCount(&cs, kSpinCount);
#endif
}

Lock::~Lock()
{
#if defined(_WIN32)
    ::DeleteCriticalSection(&cs);
#endif
}

bool Lock::TryEnter()
{
#if defined(_WIN32)
    return ::TryEnterCriticalSection(&cs) != FALSE;
#else
	return m.try_lock();
#endif
}

void Lock::Enter()
{
#if defined(_WIN32)
    ::EnterCriticalSection(&cs);
#else
	m.lock();
#endif
}

void Lock::Leave()
{
#if defined(_WIN32)
    ::LeaveCriticalSection(&cs);
#else
	m.unlock();
#endif
}


//-----------------------------------------------------------------------------
// Locker

Locker::Locker(Lock* lock) :
    TheLock(lock)
{
    if (TheLock)
        TheLock->Enter();
}

Locker::Locker(Lock& lock) :
    TheLock(&lock)
{
    if (TheLock)
        TheLock->Enter();
}

Locker::~Locker()
{
    Clear();
}

bool Locker::TrySet(Lock* lock)
{
    Clear();

    if (!lock || !lock->TryEnter())
        return false;

    TheLock = lock;
    return true;
}

bool Locker::TrySet(Lock& lock)
{
    return TrySet(&lock);
}

void Locker::Set(Lock* lock)
{
    Clear();

    if (lock)
    {
        lock->Enter();
        TheLock = lock;
    }
}

void Locker::Set(Lock& lock)
{
    return Set(&lock);
}

void Locker::Clear()
{
    if (TheLock)
    {
        TheLock->Leave();
        TheLock = nullptr;
    }
}


//-----------------------------------------------------------------------------
// AutoHandle

AutoHandle::AutoHandle(OvrLogHandle handle) :
    TheHandle(handle)
{
}

AutoHandle::~AutoHandle()
{
    Clear();
}

void AutoHandle::operator=(OvrLogHandle handle)
{
    Clear();
    TheHandle = handle;
}

void AutoHandle::Clear()
{
    if (TheHandle)
    {
        #if defined(_WIN32)
            ::CloseHandle(TheHandle);
        #else
            // To do: Implement this.
            assert(false);
        #endif
        
        TheHandle = nullptr;
    }
}


//-----------------------------------------------------------------------------
// Time

// To do: Move to using only std::chrono in the future.
// Also, it may be better to use a class instead of independent functions to
// get time and frequency.

// Returns the timer's seconds per cycle.
// Get the number to multiply by timestamps to convert to seconds.
double GetTimestampFrequencyInverse() {
  static double PerfFrequencyInverse = 0.;

  if (PerfFrequencyInverse == 0.) {
#if defined(_WIN32)
    static LARGE_INTEGER freq = {};
    if (!::QueryPerformanceFrequency(&freq) || freq.QuadPart == 0) {
      return -1.;
    }

    PerfFrequencyInverse = 1.0 / (double)freq.QuadPart;
#else
    // On OSX this period is 1 nanosecond, though that doesn't necessarily mean precision is 1
    // nanosecond.
    PerfFrequencyInverse = (double)std::chrono::high_resolution_clock::period::num /
        (double)std::chrono::high_resolution_clock::period::den;
#endif
  }

  return PerfFrequencyInverse;
}

// Returns the timer's current cycle count.
// Get current time as a timestamp
timestamp_t GetTimestamp() {
#if defined(_WIN32)
  LARGE_INTEGER timeStamp;
  if (!::QueryPerformanceCounter(&timeStamp)) {
    return 0;
  }

  return timeStamp.QuadPart;
#else
  timestamp_t t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  return t;
#endif
}

std::string GetLogFilenameDatestamp()
{
    char time_str[64];

    #ifdef _WIN32
        struct tm caltime;
        time_t now = time(0);
        localtime_s(&caltime, &now);
        strftime(time_str, 64, "%Y-%m-%d_%H.%M.%S", &caltime);
    #else
        struct tm* caltime;
        time_t now = time(0);
        caltime = localtime(&now);
        strftime(time_str, 64, "%Y-%m-%d_%H.%M.%S", caltime);
    #endif

    return time_str;
}



//-----------------------------------------------------------------------------
// Tools

bool IsDebuggerAttached()
{
    #if defined(_WIN32)
        return ::IsDebuggerPresent() != FALSE;
    #elif defined(__APPLE__)
        int               mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
        struct kinfo_proc info;
        size_t            size = sizeof(info);

        info.kp_proc.p_flag = 0;
        sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);

        return ((info.kp_proc.p_flag & P_TRACED) != 0);

    #elif defined(PT_TRACE_ME) && !defined(__android__)
        return (ptrace(PT_TRACE_ME, 0, 1, 0) < 0);
    #else
        // We have some platform-specific code elsewhere.
        #error "IsDebuggerAttached: Need to get platform-specific code for this."
    #endif
}


#if defined(_WIN32)
    static std::wstring UTF8StringToUCSString(const std::string& utf8)
    {
        #if defined(_MSC_VER) && (_MSC_VER < 1900) // VS2013
            // VS2013's wstring_convert is broken, and so we need a custom path: 
            // https://connect.microsoft.com/VisualStudio/feedback/details/796566/vc-stl-alloc-dealloc-mismatch-in-locale
            std::wstring ucs16;
            int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
            if (len > 1)
            {
                ucs16.resize(len - 1);
                wchar_t* ptr = &ucs16[0];
                MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, ptr, len);
            }
            return ucs16;
        #else
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>("", L"").from_bytes(utf8);
        #endif
    }

    static std::string UCSStringToUTF8String(const std::wstring& ucs16)
    {
        #if defined(_MSC_VER) && (_MSC_VER < 1900) // VS2013
            std::string utf8;
            int len = WideCharToMultiByte(CP_UTF8, 0, ucs16.c_str(), -1, NULL, 0, 0, 0);
            if (len > 1)
            {
                utf8.resize(len - 1);
                char* ptr = &utf8[0];
                WideCharToMultiByte(CP_UTF8, 0, ucs16.c_str(), -1, ptr, len, 0, 0);
            }
            return utf8;
        #else
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>("", L"").to_bytes(ucs16);
        #endif
    }
#endif

void ForAllLogFiles(const char* dirPath, const char* fileNamePrefix, uint64_t minAge, LogFileFunction logFileFunction)
{
    #if defined(_MSC_VER) && (_MSC_VER >= 1900) // VS2015+ only
        // <filesystem> is accepted into C++17 standard, but is already implemented in MSVC 2015.
        namespace fs = std::experimental::filesystem::v1;

        // We delete server log files that are older than N days.
        const std::wstring dirPathW = UTF8StringToUCSString(dirPath);
        const std::wstring fileNamePrefixW = UTF8StringToUCSString(fileNamePrefix);
        if (dirPathW.empty() || fileNamePrefixW.empty())
        {
            return;
        }

        const fs::path dirPathObject = dirPathW.c_str();
        fs::directory_iterator dirIterator(dirPathObject);

        for (const auto& entry : dirIterator)
        {
            if (fs::is_regular_file(entry.status()))
            {
                const std::wstring& fileNameW = entry.path().filename();

                if ((wcsstr(fileNameW.c_str(), fileNamePrefixW.c_str()) == fileNameW.c_str()))
                {
                    std::error_code errCode{}; // Pass in a default constructed error code to avoid throwing
                    auto lastWriteTime = fs::last_write_time(entry, errCode);

                    // If there was an error reading the entry, skip it
                    if (errCode)
                        continue;

                    int daysAge = std::chrono::duration_cast<std::chrono::hours>(
                        std::chrono::system_clock::now() - lastWriteTime
                    ).count() / 24;
                    if (daysAge < 0)
                    {
                        daysAge = 0;
                    }
                    const std::wstring pathStrW = dirPathW + L'\\' + fileNameW;

                    if (static_cast<uint64_t>(daysAge) > minAge) {
                        const std::string filePath = UCSStringToUTF8String(pathStrW);
                        logFileFunction(filePath.c_str(), daysAge);
                    }
                }
            }
        }
    #else
        // We delete server log files that are older than N days.
        #if defined(_WIN32)
            using namespace std::tr2::sys;

            auto ConvertFileTimeToDays = [](const FILETIME& ft) -> uint64_t 
            {
                // Convert 100ns interval to day interval.
                return (((uint64_t)ft.dwHighDateTime << 32) + ft.dwLowDateTime) / (UINT64_C(10) * 1000 * 1000 * 60 * 60 * 24); 
            };

            FILETIME currentFILETIME;
            ::GetSystemTimeAsFileTime(&currentFILETIME);
            uint64_t currentDays = ConvertFileTimeToDays(currentFILETIME);

            // Use VS2013's TR2 directory_iterator. The VS2015 version is easier to use, as it's more C++-proper.
            // This code is not C++ portable, as there is yet a C++ standard way to iterate files. However, there
            // is this: http://en.cppreference.com/w/cpp/experimental/fs/directory_iterator, which VS2015 and 
            // some other C++ libraries implement. Also, on Microsoft platforms we must use W functions instead 
            // of UTF8 functions because Windows doesn't understand UTF8.

            const std::wstring dirPathW = UTF8StringToUCSString(dirPath);
            const wpath dirPathObject = dirPathW.c_str();
            wdirectory_iterator dirIterator(dirPathObject);

            const std::wstring fileNamePrefixW = UTF8StringToUCSString(fileNamePrefix);

            while (dirIterator != wdirectory_iterator())
            {
                const auto& entry = *dirIterator;

                if (is_regular_file(entry.status()))
                {
                    const std::wstring& fileNameW = entry.path().filename();

                    if((wcsstr(fileNameW.c_str(), fileNamePrefixW.c_str()) == fileNameW.c_str()))
                    {
                        const std::wstring pathStrW = dirPathW + L'\\' + fileNameW;

                        WIN32_FIND_DATAW FindFileDataW;
                        HANDLE hFind = FindFirstFileW(pathStrW.c_str(), &FindFileDataW);

                        if (hFind != INVALID_HANDLE_VALUE) 
                        {
                            const uint64_t days = ConvertFileTimeToDays(FindFileDataW.ftLastWriteTime);
                            const uint64_t daysAge = _abs64(days - currentDays); // use abs to handle any files that have wacky dates.

                            if(daysAge > minAge)
                            {
                                const std::string filePath  = UCSStringToUTF8String(pathStrW);
                                logFileFunction(filePath.c_str(), daysAge);
                            }

                            FindClose(hFind);
                        }
                    }
                }

                ++dirIterator;
            }
        #else
            // To do: Implement this when standard C++ filesystem support is available.
            (void)dirPath;
            (void)fileNamePrefix;
            (void)minAge;
            (void)logFileFunction;
        #endif
    #endif
}



} // namespace ovrlog

#ifdef OVR_STRINGIZE
#error "This code must remain independent of LibOVR"
#endif
