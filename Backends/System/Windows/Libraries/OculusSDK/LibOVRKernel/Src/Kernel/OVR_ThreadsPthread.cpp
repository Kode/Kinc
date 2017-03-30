/************************************************************************************

Filename    :   OVR_ThreadsPthread.cpp
Content     :
Created     :
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

#if !defined(_WIN32) // Skip the entire file under Windows

#include "OVR_Threads.h"
#include "OVR_Hash.h"

#ifdef OVR_ENABLE_THREADS

#include "OVR_Timer.h"
#include "OVR_Log.h"

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#if defined(OVR_OS_MAC) || defined(OVR_OS_BSD)
    #include <sys/sysctl.h>
    #include <sys/param.h>
    #if !defined(OVR_OS_MAC)
        #include <pthread_np.h>
    #endif
#endif



namespace OVR {

// ***** Mutex implementation


// *** Internal Mutex implementation structure

class MutexImpl : public NewOverrideBase
{
    // System mutex or semaphore
    pthread_mutex_t   SMutex;
    bool          Recursive;
    unsigned      LockCount;
    pthread_t     LockedBy;

public:
    // Constructor/destructor
    MutexImpl(Mutex* pmutex, bool recursive = 1);
    ~MutexImpl();

    // Locking functions
    void                DoLock();
    bool                TryLock();
    void                Unlock(Mutex* pmutex);
    // Returns 1 if the mutes is currently locked
    bool                IsLockedByAnotherThread(Mutex* pmutex);
    bool                IsSignaled() const;
};

pthread_mutexattr_t Lock::RecursiveAttr;
bool Lock::RecursiveAttrInit = 0;

// *** Constructor/destructor
MutexImpl::MutexImpl(Mutex* pmutex, bool recursive)
{
    OVR_UNUSED(pmutex);
    Recursive           = recursive;
    LockCount           = 0;

    if (Recursive)
    {
        if (!Lock::RecursiveAttrInit)
        {
            pthread_mutexattr_init(&Lock::RecursiveAttr);
            pthread_mutexattr_settype(&Lock::RecursiveAttr, PTHREAD_MUTEX_RECURSIVE);
            Lock::RecursiveAttrInit = 1;
        }

        pthread_mutex_init(&SMutex, &Lock::RecursiveAttr);
    }
    else
        pthread_mutex_init(&SMutex, 0);
}

MutexImpl::~MutexImpl()
{
    pthread_mutex_destroy(&SMutex);
}


// Lock and try lock
void MutexImpl::DoLock()
{
    while (pthread_mutex_lock(&SMutex))
        ;
    LockCount++;
    LockedBy = pthread_self();
}

bool MutexImpl::TryLock()
{
    if (!pthread_mutex_trylock(&SMutex))
    {
        LockCount++;
        LockedBy = pthread_self();
        return 1;
    }

    return 0;
}

void MutexImpl::Unlock(Mutex* pmutex)
{
    OVR_UNUSED(pmutex);
    OVR_ASSERT(pthread_self() == LockedBy && LockCount > 0);

    //unsigned lockCount;
    LockCount--;
    //lockCount = LockCount;

    pthread_mutex_unlock(&SMutex);
}

bool    MutexImpl::IsLockedByAnotherThread(Mutex* pmutex)
{
    OVR_UNUSED(pmutex);
    // There could be multiple interpretations of IsLocked with respect to current thread
    if (LockCount == 0)
        return 0;
    if (pthread_self() != LockedBy)
        return 1;
    return 0;
}

bool    MutexImpl::IsSignaled() const
{
    // An mutex is signaled if it is not locked ANYWHERE
    // Note that this is different from IsLockedByAnotherThread function,
    // that takes current thread into account
    return LockCount == 0;
}


// *** Actual Mutex class implementation

// NOTE: RefCount mode already thread-safe for all waitables.
Mutex::Mutex(bool recursive)
    : pImpl(new MutexImpl(this, recursive)) { }

Mutex::~Mutex() { }

// Lock and try lock
void Mutex::DoLock()
{
    pImpl->DoLock();
}
bool Mutex::TryLock()
{
    return pImpl->TryLock();
}
void Mutex::Unlock()
{
    pImpl->Unlock(this);
}
bool    Mutex::IsLockedByAnotherThread()
{
    return pImpl->IsLockedByAnotherThread(this);
}



//-----------------------------------------------------------------------------------
// ***** Event

bool Event::Wait(unsigned delay)
{
    std::unique_lock<std::mutex> locker(StateMutex);

    // Do the correct amount of waiting
    if (delay == OVR_WAIT_INFINITE)
    {
        while(!State)
            StateWaitCondition.wait(locker);
    }
    else if (delay)
    {
        if (!State)
            StateWaitCondition.wait_for(locker,
                std::chrono::milliseconds(delay));
    }

    bool state = State;
    // Take care of temporary 'pulsing' of a state
    if (Temporary)
    {
        Temporary   = false;
        State       = false;
    }
    return state;
}

void Event::updateState(bool newState, bool newTemp, bool mustNotify)
{
    std::unique_lock<std::mutex> lock(StateMutex);
    State       = newState;
    Temporary   = newTemp;
    if (mustNotify)
    {
        lock.unlock();
        StateWaitCondition.notify_all();
    }
}

ThreadId GetCurrentThreadId()
{
    return (void*)pthread_self();
}

// *** Sleep functions

/* static */
bool    Thread::Sleep(unsigned secs)
{
    sleep(secs);
    return 1;
}
/* static */
bool    Thread::MSleep(unsigned msecs)
{
    usleep(msecs*1000);
    return 1;
}

void Thread::SetCurrentThreadName(const char* name)
{
    #if defined (OVR_OS_APPLE)
        pthread_setname_np(name);
    #else
        pthread_setname_np(pthread_self(), name);
    #endif
}

void Thread::GetCurrentThreadName(char* name, size_t nameCapacity)
{
    name[0] = 0;
#if !defined (OVR_OS_ANDROID)
    // Android does not have pthread_getname_np (or an equivalent)
    pthread_getname_np(pthread_self(), name, nameCapacity);
#endif
}


} // namespace OVR

#endif  // OVR_ENABLE_THREADS

#endif // _WIN32
