/************************************************************************************

Filename    :   OVR_ThreadsWinAPI.cpp
Platform    :   WinAPI
Content     :   Windows specific thread-related (safe) functionality
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

#include "OVR_Threads.h"
#include "OVR_Hash.h"
#include "OVR_Log.h"
#include "OVR_Timer.h"

#ifdef OVR_ENABLE_THREADS

// For _beginthreadex / _endtheadex
#include <process.h>

namespace OVR {

//-----------------------------------------------------------------------------------
// *** Internal Mutex implementation class

class MutexImpl : public NewOverrideBase {
  // System mutex or semaphore
  HANDLE hMutexOrSemaphore;
  bool Recursive;
  volatile unsigned LockCount;

 public:
  // Constructor/destructor
  MutexImpl(bool recursive = 1);
  ~MutexImpl();

  // Locking functions
  void DoLock();
  bool TryLock();
  void Unlock(Mutex* pmutex);
  // Returns 1 if the mutes is currently locked
  bool IsLockedByAnotherThread(Mutex* pmutex);
};

// *** Constructor/destructor
MutexImpl::MutexImpl(bool recursive) {
  Recursive = recursive;
  LockCount = 0;
#if defined(OVR_OS_WIN32) // Older versions of Windows don't support CreateSemaphoreEx, so stick
  // with CreateSemaphore for portability.
  hMutexOrSemaphore = Recursive ? CreateMutexW(NULL, 0, NULL) : CreateSemaphoreW(NULL, 1, 1, NULL);
#else
  // No CreateSemaphore() call, so emulate it.
  hMutexOrSemaphore = Recursive ? CreateMutexW(NULL, 0, NULL)
                                : CreateSemaphoreExW(NULL, 1, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
#endif
}
MutexImpl::~MutexImpl() {
  CloseHandle(hMutexOrSemaphore);
}

// Lock and try lock
void MutexImpl::DoLock() {
  if (::WaitForSingleObject(hMutexOrSemaphore, INFINITE) != WAIT_OBJECT_0)
    return;
  LockCount++;
}

bool MutexImpl::TryLock() {
  DWORD ret;
  if ((ret = ::WaitForSingleObject(hMutexOrSemaphore, 0)) != WAIT_OBJECT_0)
    return 0;
  LockCount++;
  return 1;
}

void MutexImpl::Unlock(Mutex* pmutex) {
  OVR_UNUSED(pmutex);

  unsigned lockCount;
  LockCount--;
  lockCount = LockCount;

  // Release mutex
  if ((Recursive ? ReleaseMutex(hMutexOrSemaphore)
                 : ReleaseSemaphore(hMutexOrSemaphore, 1, NULL)) != 0) {
    // This used to call Wait handlers if lockCount == 0.
  }
}

bool MutexImpl::IsLockedByAnotherThread(Mutex* pmutex) {
  // There could be multiple interpretations of IsLocked with respect to current thread
  if (LockCount == 0)
    return 0;
  if (!TryLock())
    return 1;
  Unlock(pmutex);
  return 0;
}

/*
bool    MutexImpl::IsSignaled() const
{
    // An mutex is signaled if it is not locked ANYWHERE
    // Note that this is different from IsLockedByAnotherThread function,
    // that takes current thread into account
    return LockCount == 0;
}
*/

// *** Actual Mutex class implementation

Mutex::Mutex(bool recursive) : pImpl(new MutexImpl(recursive)) {}

Mutex::~Mutex() {}

// Lock and try lock
void Mutex::DoLock() {
  pImpl->DoLock();
}
bool Mutex::TryLock() {
  return pImpl->TryLock();
}
void Mutex::Unlock() {
  pImpl->Unlock(this);
}
bool Mutex::IsLockedByAnotherThread() {
  return pImpl->IsLockedByAnotherThread(this);
}

//-----------------------------------------------------------------------------------
// ***** Event

bool Event::Wait(unsigned delay) {
  std::unique_lock<std::mutex> locker(StateMutex);

  // Do the correct amount of waiting
  if (delay == OVR_WAIT_INFINITE) {
    while (!State)
      StateWaitCondition.wait(locker);
  } else if (delay) {
    if (!State)
      StateWaitCondition.wait_for(locker, std::chrono::milliseconds(delay));
  }

  bool state = State;
  // Take care of temporary 'pulsing' of a state
  if (Temporary) {
    Temporary = false;
    State = false;
  }
  return state;
}

void Event::updateState(bool newState, bool newTemp, bool mustNotify) {
  {
    std::lock_guard<std::mutex> lock(StateMutex);
    State = newState;
    Temporary = newTemp;
  }

  // NOTE: The lock does not need to be held when calling notify_all(),
  // and holding it is in fact a pessimization.
  if (mustNotify)
    StateWaitCondition.notify_all();
}

//-----------------------------------------------------------------------------------
// ***** Thread Namespace

// *** Sleep functions

// static
bool Thread::MSleep(unsigned msecs) {
  ::Sleep(msecs);
  return 1;
}

static OVR_THREAD_LOCAL char ThreadLocaThreadlName[32] = {};

void Thread::SetCurrentThreadName(const char* name) {
  OVR_strlcpy(ThreadLocaThreadlName, name, sizeof(ThreadLocaThreadlName));

#if !defined(OVR_BUILD_SHIPPING) || defined(OVR_BUILD_PROFILING)
// http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#pragma pack(push, 8)
  struct THREADNAME_INFO {
    DWORD dwType; // Must be 0x1000
    LPCSTR szName; // Pointer to name (in user address space)
    DWORD dwThreadID; // Thread ID (-1 for caller thread)
    DWORD dwFlags; // Reserved for future use; must be zero
  };
  union TNIUnion {
    THREADNAME_INFO tni;
    ULONG_PTR upArray[4];
  };
#pragma pack(pop)

  TNIUnion tniUnion = {0x1000, name, ::GetCurrentThreadId(), 0};

  __try {
    RaiseException(0x406D1388, 0, OVR_ARRAY_COUNT(tniUnion.upArray), tniUnion.upArray);
  } __except (
      GetExceptionCode() == 0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER) {
    return;
  }
#endif // OVR_BUILD_SHIPPING
}

void Thread::GetCurrentThreadName(char* name, size_t nameCapacity) {
  OVR_strlcpy(name, ThreadLocaThreadlName, nameCapacity);
}

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId() {
  return (ThreadId)::GetCurrentThreadId();
}

} // namespace OVR

#endif
