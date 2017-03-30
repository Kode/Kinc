/************************************************************************************

Filename    :   OVR_Atomic.h
Content     :   Contains atomic operations and inline fastest locking
                functionality. Will contain #ifdefs for OS efficiency.
                Have non-thread-safe implementaion if not available.
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

#ifndef OVR_Atomic_h
#define OVR_Atomic_h

#include "OVR_Types.h"

#include <atomic>

// Include System thread functionality.
#if defined(OVR_OS_MS) && !defined(OVR_OS_MS_MOBILE)
#include "OVR_Win32_IncludeWindows.h"
#else
#include <pthread.h>
#endif

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** Lock

// Lock is a simplest and most efficient mutual-exclusion lock class.
// Unlike Mutex, it cannot be waited on.

class Lock
{    
#if !defined(OVR_ENABLE_THREADS)

public:
    // With no thread support, lock does nothing.
    inline Lock() { }
    inline Lock(unsigned) { }
    inline ~Lock() { }    
    inline void DoLock() { }
    inline void Unlock() { }

   // Windows.   
#elif defined(OVR_OS_MS)

    CRITICAL_SECTION cs;
public:   
    Lock(unsigned spinCount = 10000);   // Mutexes with non-zero spin counts usually result in better performance.
    ~Lock();
    // Locking functions.
    inline void DoLock()    { ::EnterCriticalSection(&cs); }
    inline void Unlock()    { ::LeaveCriticalSection(&cs); }
    inline bool TryLock()   { return (::TryEnterCriticalSection(&cs) == TRUE); }
#else
    pthread_mutex_t mutex;

public:
    static pthread_mutexattr_t RecursiveAttr;
    static bool                RecursiveAttrInit;

    Lock (unsigned spinCount = 0) // To do: Support spin count, probably via a custom lock implementation.
    {
        OVR_UNUSED(spinCount);
        if (!RecursiveAttrInit)
        {
            pthread_mutexattr_init(&RecursiveAttr);
            pthread_mutexattr_settype(&RecursiveAttr, PTHREAD_MUTEX_RECURSIVE);
            RecursiveAttrInit = 1;
        }
        pthread_mutex_init(&mutex,&RecursiveAttr);
    }
    ~Lock ()                { pthread_mutex_destroy(&mutex); }
    inline void DoLock()    { pthread_mutex_lock(&mutex); }
    inline void Unlock()    { pthread_mutex_unlock(&mutex); }

#endif // OVR_ENABLE_THREDS


public:
    // Locker class, used for automatic locking
    class Locker
    {
        Lock *pLock;

    public:
        Locker(Lock *plock)
        {
            pLock = plock;
            if (plock)
                pLock->DoLock();
        }
        ~Locker()
        {
            Release();
        }

        void Release()
        {
            if (pLock)
                pLock->Unlock();
            pLock = nullptr;
        }
    };

    // Unlocker class, used for automatic unlocking
    class Unlocker
    {
        //OVR_NON_COPYABLE(Unlocker);
        Lock* mLock;

    public:
        Unlocker(Lock *lock) : mLock(lock) { }
        ~Unlocker() { Release(); }
        void Release()
        {
            if (mLock) mLock->Unlock();
            mLock = nullptr;
        }
    };
};


//-------------------------------------------------------------------------------------
// Globally shared Lock implementation used for MessageHandlers, etc.

class SharedLock
{    
public:
    SharedLock() : UseCount(0) {}

    Lock* GetLockAddRef();
    void  ReleaseLock(Lock* plock);
   
private:
    Lock* toLock() { return (Lock*)Buffer; }

    // UseCount and max alignment.
    std::atomic<int>  UseCount;
    uint64_t        Buffer[(sizeof(Lock)+sizeof(uint64_t)-1)/sizeof(uint64_t)];
};


//-------------------------------------------------------------------------------------
// Thin locking wrapper around data

template<class T> class LockedData
{
public:
    LockedData(Lock& lock) :
        TheLock(lock)
    {
    }
    LockedData& operator=(const LockedData& rhs)
    {
        OVR_ASSERT(false);
        return *this;
    }

    T Get()
    {
        Lock::Locker locker(&TheLock);
        return Instance;
    }

    void Set(const T& value)
    {
        Lock::Locker locker(&TheLock);
        Instance = value;
    }

    // Returns true if the value has changed.
    // Returns false if the value has not changed.
    bool GetIfChanged(T& value)
    {
        Lock::Locker locker(&TheLock);

        if (value != Instance)
        {
            value = Instance;
            return true;
        }

        return false;
    }

protected:
    T Instance;
    Lock& TheLock;
};


} // namespace OVR

#endif
