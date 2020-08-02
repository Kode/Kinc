/************************************************************************************

Filename    :   OVR_System.cpp
Content     :   General kernel initialization/cleanup, including that
                of the memory allocator.
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

#include "OVR_System.h"
#include "OVR_Threads.h"
#include "OVR_Timer.h"
#include "OVR_DebugHelp.h"
#include "OVR_Log.h"
#include <new>

#if defined(_MSC_VER)
#include <new.h>
#else
#include <new>
#endif

#ifdef OVR_OS_MS
#pragma warning(push, 0)
#include "OVR_Win32_IncludeWindows.h" // GetModuleHandleEx
#pragma warning(pop)
#endif

static ovrlog::Channel Logger("Kernel:System");

namespace OVR {

//-----------------------------------------------------------------------------
// Initialization/Shutdown state
// If true, then Destroy() was called and is in the process of executing
// Added to fix race condition if thread is started after the call to System::Destroy()
static bool ShuttingDown = false;

//-----------------------------------------------------------------------------
// Initialization/Shutdown Callbacks

static SystemSingletonInternal* SystemShutdownListenerList =
    nullptr; // Points to the most recent SystemSingletonInternal added to the list.

static Lock& GetSSILock() { // Put listLock in a function so that it can be constructed on-demand.
  static Lock listLock; // Will construct on the first usage. However, the guarding of this
  // construction is not thread-safe
  return listLock; // under all compilers. However, since we are initially calling this on startup
  // before other threads
} // could possibly exist, the first usage of this will be before other threads exist.

void SystemSingletonInternal::RegisterDestroyCallback() {
  GetSSILock().DoLock();
  if (ShuttingDown) {
    GetSSILock().Unlock();

    OnThreadDestroy();
  } else {
    GetSSILock().Unlock();

    // Insert the listener at the front of the list (top of the stack). This is an analogue of a C++
    // forward_list::push_front or stack::push.
    NextShutdownSingleton = SystemShutdownListenerList;
    SystemShutdownListenerList = this;
  }
}

//-----------------------------------------------------------------------------
// System

static int System_Init_Count = 0;

#if defined(_MSC_VER)
// This allows us to throw OVR::bad_alloc instead of std::bad_alloc, which provides less
// information.
int OVRNewFailureHandler(size_t /*size*/) {
  throw OVR::bad_alloc();

  // Disabled because otherwise a compiler warning is generated regarding unreachable code.
  // return 0; // A return value of 0 tells the Standard Library to not retry the allocation.
}
#else
// This allows us to throw OVR::bad_alloc instead of std::bad_alloc, which provides less
// information.
void OVRNewFailureHandler() {
  throw OVR::bad_alloc();
}
#endif

// Initializes System core, installing allocator.
void System::Init() {
  // Restart logging if we shut down before
  ovrlog::RestartLogging();

#if defined(_MSC_VER)
  // Make it so that failure of the C malloc family of functions results in the same behavior as C++
  // operator new failure.
  // This allows us to throw exceptions for malloc usage the same as for operator new bad_alloc.
  _set_new_mode(1);

  // Tells the standard library to direct new (and malloc) failures to us. Normally we wouldn't need
  // to do this, as the
  // C++ Standard Library already throws std::bad_alloc on operator new failure. The problem is that
  // the Standard Library doesn't
  // throw std::bad_alloc upon malloc failure, and we can only intercept malloc failure via this
  // means. _set_new_handler specifies
  // a global handler for the current running Standard Library. If the Standard Library is being
  // dynamically linked instead
  // of statically linked, then this is a problem because a call to _set_new_handler would override
  // anything the application
  // has already set.
  _set_new_handler(OVRNewFailureHandler);
#else
  // This allows us to throw OVR::bad_alloc instead of std::bad_alloc, which provides less
  // information.
  // Question: Does this set the handler for all threads or just the current thread? The C++
  // Standard doesn't
  // explicitly state this, though it may be implied from other parts of the Standard.
  std::set_new_handler(OVRNewFailureHandler);
#endif

  if (++System_Init_Count == 1) {
    Timer::initializeTimerSystem();
  } else {
    Logger.LogError("Init recursively called; depth = ", System_Init_Count);
    // XXX Should this just exit?
  }
}

void System::Stop() {
  GetSSILock().DoLock();
  ShuttingDown = true;
  GetSSILock().Unlock();

  if (--System_Init_Count == 0) {
    Logger.LogInfo("Graceful shutdown: OnThreadDestroy");

    // Invoke all of the post-finish callbacks (normal case)
    for (SystemSingletonInternal* listener = SystemShutdownListenerList; listener;
         listener = listener->NextShutdownSingleton) {
      listener->OnThreadDestroy();
    }
  } else {
    Logger.LogError("Stop recursively called; depth = ", System_Init_Count);
  }
}

void System::Destroy() {
  if (!ShuttingDown) {
    Logger.LogWarning("Destroy called before Stop");
    System::Stop();
  }

  if (System_Init_Count == 0) {
    Logger.LogInfo("Graceful shutdown: OnSystemDestroy");

    // Invoke all of the post-finish callbacks (normal case)
    for (SystemSingletonInternal *next, *listener = SystemShutdownListenerList; listener;
         listener = next) {
      next = listener->NextShutdownSingleton;
      listener->OnSystemDestroy();
    }

    SystemShutdownListenerList = nullptr;

    Timer::shutdownTimerSystem();
  } else {
    Logger.LogError("Destroy recursively called; depth = ", System_Init_Count);
  }

  GetSSILock().DoLock();
  ShuttingDown = false;
  GetSSILock().Unlock();

  Logger.LogInfo("Graceful shutdown: Stopping logger");

  // Prevent memory leak reports
  ovrlog::ShutdownLogging();
}

// Returns 'true' if system was properly initialized.
bool System::IsInitialized() {
  return System_Init_Count > 0;
}

// Dump any leaked memory
void System::CheckForAllocatorLeaks() {
  if (Allocator::IsTrackingLeaks()) {
    int ovrLeakCount = Allocator::DumpMemory();
    (void)ovrLeakCount;

    OVR_ASSERT(ovrLeakCount == 0);
  }
}

} // namespace OVR
