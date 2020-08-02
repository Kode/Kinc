/************************************************************************************

PublicHeader:   None
Filename    :   OVR_Threads.h
Content     :   Contains thread-related (safe) functionality
Created     :   September 19, 2012
Notes       :

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

************************************************************************************/
#ifndef OVR_Threads_h
#define OVR_Threads_h

#include "OVR_Types.h"
#include "OVR_Atomic.h"
#include "OVR_RefCount.h"
#include "OVR_Array.h"

#include <memory>
#include <mutex>
#include <condition_variable>

#if !defined(_WIN32)
#include <unistd.h>
#endif

// Defines the infinite wait delay timeout
#define OVR_WAIT_INFINITE 0xFFFFFFFF

// To be defined in the project configuration options
#ifdef OVR_ENABLE_THREADS

namespace OVR {

//-----------------------------------------------------------------------------------
// ****** Declared classes

// Declared with thread support only
class Mutex;
class Event;
// Implementation forward declarations
class MutexImpl;

//-----------------------------------------------------------------------------------
// ***** Mutex

// Mutex class represents a system Mutex synchronization object that provides access
// serialization between different threads, allowing one thread mutually exclusive access
// to a resource. Mutex is more heavy-weight then Lock, but supports WaitCondition.

class Mutex {
  friend class MutexImpl;

  std::unique_ptr<MutexImpl> pImpl;

 public:
  // Constructor/destructor
  Mutex(bool recursive = 1);
  ~Mutex();

  // Locking functions
  void DoLock();
  bool TryLock();
  void Unlock();

  // Returns 1 if the mutes is currently locked by another thread
  // Returns 0 if the mutex is not locked by another thread, and can therefore be acquired.
  bool IsLockedByAnotherThread();

  // Locker class; Used for automatic locking of a mutex withing scope
  class Locker {
   public:
    Mutex* pMutex;
    Locker(Mutex* pmutex) : pMutex(pmutex) {
      pMutex->DoLock();
    }
    Locker(const std::unique_ptr<Mutex>& pmutex) : Locker(pmutex.get()) {}
    ~Locker() {
      pMutex->Unlock();
    }
  };
};

//-----------------------------------------------------------------------------------
// ***** Event

// Event is a wait-able synchronization object similar to Windows event.
// Event can be waited on until it's signaled by another thread calling
// either SetEvent or PulseEvent.

class Event {
  // Event state, its mutex and the wait condition
  volatile bool State;
  volatile bool Temporary;
  mutable std::mutex StateMutex;
  std::condition_variable StateWaitCondition;

  void updateState(bool newState, bool newTemp, bool mustNotify);

 public:
  Event(bool setInitially = 0) : State(setInitially), Temporary(false) {}
  ~Event() {}

  // Wait on an event condition until it is set
  // Delay is specified in milliseconds (1/1000 of a second).
  bool Wait(unsigned delay = OVR_WAIT_INFINITE);

  // Set an event, releasing objects waiting on it
  void SetEvent() {
    updateState(true, false, true);
  }

  // Reset an event, un-signaling it
  void ResetEvent() {
    updateState(false, false, false);
  }

  // Set and then reset an event once a waiter is released.
  // If threads are already waiting, they will be notified and released
  // If threads are not waiting, the event is set until the first thread comes in
  void PulseEvent() {
    updateState(true, true, true);
  }
};

//-----------------------------------------------------------------------------------
// ***** Thread class

// ThreadHandle is a handle to a thread, which on some platforms (e.g. Windows) is
// different from ThreadId. On Unix platforms, a ThreadHandle is the same as a
// ThreadId and is pthread_t.
typedef void* ThreadHandle;

// ThreadId uniquely identifies a thread; returned by Windows GetCurrentThreadId(),
// Unix pthread_self() and Thread::GetThreadId.
typedef void* ThreadId;

// *** Thread flags

// Indicates that the thread is has been started, i.e. Start method has been called, and threads
// OnExit() method has not yet been called/returned.
#define OVR_THREAD_STARTED 0x01
// This flag is set once the thread has ran, and finished.
#define OVR_THREAD_FINISHED 0x02
// This flag is set temporarily if this thread was started suspended. It is used internally.
#define OVR_THREAD_START_SUSPENDED 0x08
// This flag is used to ask a thread to exit. Message driven threads will usually check this flag
// and finish once it is set.
#define OVR_THREAD_EXIT 0x10

namespace Thread {
// *** Sleep

// Sleep msecs milliseconds
bool MSleep(unsigned msecs);

// *** Debugging functionality
void SetCurrentThreadName(const char* name);
void GetCurrentThreadName(char* name, size_t nameCapacity);
}; // namespace Thread

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId();

// Returns the unique Id of the current running process.
#if !defined(OVR_OS_MS)
#define GetCurrentProcessId getpid
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_THREAD_LOCAL
//
// Example usage:
//    #if defined(OVR_THREAD_LOCAL)
//        OVR_THREAD_LOCAL int n = 0;                       // OK
//        extern OVR_THREAD_LOCAL struct Data s;            // OK
//        static OVR_THREAD_LOCAL char* p;                  // OK
//        OVR_THREAD_LOCAL int i = sizeof(i);               // OK.
//        OVR_THREAD_LOCAL std::string s("hello");          // Can't be used for initialized
//        objects.
//        OVR_THREAD_LOCAL int Function();                  // Can't be used as return value.
//        void Function(){ OVR_THREAD_LOCAL int i = 0; }    // Can't be used in function.
//        void Function(OVR_THREAD_LOCAL int i){ }          // can't be used as argument.
//        extern int i; OVR_THREAD_LOCAL int i;             // Declarations differ.
//        int OVR_THREAD_LOCAL i;                           // Can't be used as a type modifier.
//        OVR_THREAD_LOCAL int i = i;                       // Can't reference self before
//        initialization.
//    #endif

#if !defined(_MSC_VER) || \
    (_MSC_VER >= 1900) // VC++ doesn't support C++11 thread_local storage until VS2015.
#define OVR_THREAD_LOCAL thread_local
#else
#define OVR_THREAD_LOCAL __declspec(thread)
#endif

} // namespace OVR

#endif // OVR_ENABLE_THREADS
#endif // OVR_Threads_h
