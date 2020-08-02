/************************************************************************************

Filename    :   OVR_Atomic.cpp
Content     :   Contains atomic operations and inline fastest locking
                functionality. Will contain #ifdefs for OS efficiency.
                Have non-thread-safe implementation if not available.
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

#include "OVR_Atomic.h"
#include "OVR_Allocator.h"

#ifdef OVR_ENABLE_THREADS

// Include Windows 8-Metro compatible Synchronization API
#if defined(OVR_OS_MS) && defined(NTDDI_WIN8) && (NTDDI_VERSION >= NTDDI_WIN8)
#include <synchapi.h>
#endif

namespace OVR {

// ***** Windows Lock implementation

#if defined(OVR_OS_MS)

// ***** Standard Win32 Lock implementation

// Constructors
Lock::Lock(unsigned spinCount) {
#if defined(NTDDI_WIN8) && (NTDDI_VERSION >= NTDDI_WIN8)
  // On Windows 8 we use InitializeCriticalSectionEx due to Metro-Compatibility
  InitializeCriticalSectionEx(
      &cs, (DWORD)spinCount, OVR_DEBUG_SELECT(NULL, CRITICAL_SECTION_NO_DEBUG_INFO));
#else
  ::InitializeCriticalSectionAndSpinCount(
      &cs, (DWORD)spinCount); // This is available with WindowsXP+.
#endif
}

Lock::~Lock() {
  DeleteCriticalSection(&cs);
}

#endif

//-------------------------------------------------------------------------------------
// ***** SharedLock

// This is a general purpose globally shared Lock implementation that should probably be
// moved to Kernel.
// May in theory busy spin-wait if we hit contention on first lock creation,
// but this shouldn't matter in practice since Lock* should be cached.

enum { LockInitMarker = 0xFFFFFFFF };

Lock* SharedLock::GetLockAddRef() {
  int oldUseCount, oldUseCount_tmp;

  do {
    oldUseCount = UseCount;
    if (oldUseCount == (int)LockInitMarker)
      continue;

    if (oldUseCount == 0) {
      // Initialize marker
      int tmp_zero = 0;
      int tmp_LockInitMarker = LockInitMarker;
      if (UseCount.compare_exchange_strong(tmp_zero, LockInitMarker)) {
        Construct<Lock>(Buffer);
        do {
        } while (UseCount.compare_exchange_weak(tmp_LockInitMarker, 1));
        return toLock();
      }
      continue;
    }
    oldUseCount_tmp = oldUseCount;
  } while (
      !UseCount.compare_exchange_weak(oldUseCount_tmp, oldUseCount + 1, std::memory_order_relaxed));

  return toLock();
}

void SharedLock::ReleaseLock(Lock* plock) {
  OVR_UNUSED(plock);
  OVR_ASSERT(plock == toLock());

  int oldUseCount, oldUseCount_tmp;

  do {
    oldUseCount = UseCount;
    OVR_ASSERT(oldUseCount != (int)LockInitMarker);

    if (oldUseCount == 1) {
      // Initialize marker
      int tmp_one = 1;
      int tmp_LockInitMarker = LockInitMarker;
      if (UseCount.compare_exchange_strong(tmp_one, LockInitMarker)) {
        Destruct<Lock>(toLock());

        do {
        } while (!UseCount.compare_exchange_weak(tmp_LockInitMarker, 0));

        return;
      }
      continue;
    }

    oldUseCount_tmp = oldUseCount;
  } while (
      !UseCount.compare_exchange_weak(oldUseCount_tmp, oldUseCount - 1, std::memory_order_relaxed));
}

} // namespace OVR

#endif // OVR_ENABLE_THREADS
