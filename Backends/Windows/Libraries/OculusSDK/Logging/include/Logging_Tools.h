/************************************************************************************

Filename    :   Logging_Tools.h
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

#ifndef Logging_Tools_h
#define Logging_Tools_h

#include <string>


//-----------------------------------------------------------------------------
// Platform-specific macros

#ifdef _MSC_VER
    #define LOGGING_INLINE __forceinline
#else
    #define LOGGING_INLINE inline
#endif

#if defined(_DEBUG) || defined(DEBUG)
    #define LOGGING_DEBUG
#endif

#define LOGGING_STRINGIZEIMPL(x) #x
#define LOGGING_STRINGIZE(x) LOGGING_STRINGIZEIMPL(x)

#if defined(_WIN32)
    // It is common practice to define WIN32_LEAN_AND_MEAN to reduce compile times.
    // However this then requires us to define our own NTSTATUS data type and other
    // irritations throughout our code-base.
    #ifndef WIN32_LEAN_AND_MEAN
        //#define WIN32_LEAN_AND_MEAN
    #endif

    // Prevents <Windows.h> from #including <Winsock.h>, as we use <Winsock2.h> instead.
    #ifndef _WINSOCKAPI_
        #define DID_DEFINE_WINSOCKAPI
        #define _WINSOCKAPI_
    #endif

    // Prevents <Windows.h> from defining min() and max() macro symbols.
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    // We support Windows Windows 7 or newer.
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601 /* Windows 7+ */
    #endif

#define WIN
    #include <windows.h>
#endif

    #ifdef DID_DEFINE_WINSOCKAPI
        #undef _WINSOCKAPI_
        #undef DID_DEFINE_WINSOCKAPI
    #endif

#if defined(_MSC_VER) && defined(LOGGING_DEBUG)
    #include <intrin.h>
    #define LOGGING_DEBUG_BREAK() __debugbreak()
#else
    #define LOGGING_DEBUG_BREAK()
#endif

#include <atomic>

namespace ovrlog {


//-----------------------------------------------------------------------------
// Lock
//
// Critical section wrapper.
class Lock
{
public:
    Lock();
    ~Lock();

    // Returns true if lock could be held.
    bool TryEnter();

    void Enter();
    void Leave();

private:
    CRITICAL_SECTION cs;
};


//-----------------------------------------------------------------------------
// Locker
//
// Scoped lock wrapper.
class Locker
{
public:
    Locker(Lock* lock = nullptr);
    Locker(Lock& lock);
    ~Locker();

    // Returns true if lock could be held.
    bool TrySet(Lock* lock);
    bool TrySet(Lock& lock);

    // Lock the given lock.  Unlocks previously held lock.
    void Set(Lock* lock);
    void Set(Lock& lock);

    // Unlock any previously held lock.
    void Clear();

private:
    Lock* TheLock;
};


//-----------------------------------------------------------------------------
// AutoHandle
//
// Auto-close wrapper for a HANDLE that is invalid when NULL.
// For example, ::OpenProcess() returns NULL on failure.
class AutoHandle
{
public:
    AutoHandle(HANDLE handle = nullptr);
    ~AutoHandle();

    void operator=(HANDLE handle);

    HANDLE Get() const
    {
        return TheHandle;
    }

    bool IsValid() const
    {
        return TheHandle != nullptr;
    }

    void Clear();

protected:
    HANDLE TheHandle;
};


//-----------------------------------------------------------------------------
// Terminator
//
// Helper class that allows for signaled exits to an infinite event wait.
// The main purpose is the WaitOn() function.
class Terminator
{
public:
    Terminator();
    ~Terminator();

    bool IsTerminated() const { return Terminated; }

    // Setup
    bool Initialize();

    // Flag terminated
    void Terminate();

    // Returns true if the event signaled and false on termination or timeout.
    // Call IsTerminated() to differentiate termination from timeout.
    // Passing INFINITE for timeout will only return false on termination.
    bool WaitOn(HANDLE hEvent, DWORD timeoutMsec = INFINITE);

    // Returns true if the sleep interval exceeded or false on termination.
    bool WaitSleep(int milliseconds);

private:
    // Should we terminate?
    std::atomic<bool> Terminated;

    // Event to wake up during waits
    AutoHandle TerminateEvent;
};


//-----------------------------------------------------------------------------
// Time

typedef unsigned long long timestamp_t;

static_assert(sizeof(timestamp_t) == 8, "64-bit types not supported?");

// Get current time as a timestamp
timestamp_t GetTimestamp();

// Get the number to multiply by timestamps to convert to seconds
double GetTimestampFrequencyInverse();

// Get time in seconds. The time is not based on any absolute reference point,
// but rather is based on a platform-specific and runtime-specific start time.
inline double GetTimeSeconds()
{
    return GetTimestampFrequencyInverse() * GetTimestamp();
}

// Returns a string like: "2015-12-08_16.41.02", for use in the naming of log files.
std::string GetLogFilenameDatestamp();



//-----------------------------------------------------------------------------
// Tools

bool IsDebuggerAttached();


// Calls the logFileFunction for each file in the dirPath (non-recursively), with the 
// given fileNamePrefix, and which was last written to at least 'age' days ago. This is 
// useful for purging old log files, for example.
//
// Example usage:
//     ForAllLogFiles("C:\\SomeDir", "ServerLog_", 5, [](const char* filePath, uint64_t age)->void{ DeleteFile(filePath); });
//
typedef void (*LogFileFunction)(const char* filePath, uint64_t age);
void ForAllLogFiles(const char* dirPath, const char* fileNamePrefix, uint64_t minAge, LogFileFunction logFileFunction);


} // namespace ovrlog

#endif // Logging_Tools_h
