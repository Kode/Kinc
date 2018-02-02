/************************************************************************************

PublicHeader:   Kernel
Filename    :   OVR_CallbacksInternal.h
Content     :   Callback library
Created     :   Nov 11, 2014
Author      :   Chris Taylor

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

#ifndef OVR_CallbacksInternal_h
#define OVR_CallbacksInternal_h

#include "OVR_Atomic.h"
#include "OVR_RefCount.h"
#include "OVR_Delegates.h"
#include "OVR_Array.h"

#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
#include <atomic>
#endif

namespace OVR {

template <class DelegateT>
class FloatingCallbackEmitter; // Floating emitter object
template <class DelegateT>
class CallbackEmitter;
template <class DelegateT>
class FloatingCallbackListener; // Floating listener object
template <class DelegateT>
class CallbackListener;

//-----------------------------------------------------------------------------
// FloatingCallbackEmitter
//
// The Call() function is not thread-safe.
// TBD: Should we add a thread-safe Call() option to constructor?

class CallbackEmitterBase {
 protected:
  static Lock* GetEmitterLock();
};

template <class DelegateT>
class FloatingCallbackEmitter : public CallbackEmitterBase,
                                public RefCountBase<FloatingCallbackEmitter<DelegateT>> {
  friend class CallbackEmitter<DelegateT>;

  FloatingCallbackEmitter()
      : IsShutdown(false),
#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
        ListenersExist(false),
#endif
        DirtyListenersCache(0) {
  }

 public:
  typedef Array<Ptr<FloatingCallbackListener<DelegateT>>> ListenerPtrArray;

  ~FloatingCallbackEmitter() {
    OVR_ASSERT(Listeners.GetSizeI() == 0);
    // ListenersCache will be emptied here.
  }

  bool AddListener(FloatingCallbackListener<DelegateT>* listener);
  bool HasListeners() const {
#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
    return ListenersExist.load(std::memory_order_relaxed);
#else
    // This code still has a data race
    return (Listeners.GetSizeI() > 0);
#endif
  }
  void Shutdown();

  // Called from the listener object as it is transitioning to canceled state.
  // The listener's mutex is not held during this call.
  void OnListenerCancel(FloatingCallbackListener<DelegateT>* listener);

 public:
  void Call();

  template <class Param1>
  void Call(Param1* p1);

  template <class Param1>
  void Call(Param1& p1);

  template <class Param1, class Param2>
  void Call(Param1* p1, Param2* p2);

  template <class Param1, class Param2>
  void Call(Param1& p1, Param2& p2);

  template <class Param1, class Param2, class Param3>
  void Call(Param1* p1, Param2* p2, Param3* p3);

  template <class Param1, class Param2, class Param3>
  void Call(Param1& p1, Param2& p2, Param3& p3);

 protected:
  // Is the emitter shut down?  This prevents more listeners from being added during shutdown.
  bool IsShutdown;

  // Array of added listeners.
  ListenerPtrArray Listeners;

#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
  std::atomic<bool> ListenersExist;
#endif

  // Is the cache dirty?  This avoids locking and memory allocation in steady state.
  std::atomic<uint32_t> DirtyListenersCache = {0};

  // Cache of listeners used by the Call() function.
  ListenerPtrArray ListenersCacheForCalls;

  // Update the ListenersCache array in response to an insertion or removal.
  // This is how AddListener() insertions get rolled into the listeners array.
  // This is how RemoveListener() removals get purged from the cache.
  void updateListenersCache() {
    if (DirtyListenersCache != 0) {
      Lock::Locker locker(GetEmitterLock());

      // TBD: Should memory allocation be further reduced here?
      ListenersCacheForCalls = Listeners;
      DirtyListenersCache = 0;
    }
  }

  // Without holding a lock, find and remove the given listener from the array of listeners.
  void noLockFindAndRemoveListener(FloatingCallbackListener<DelegateT>* listener) {
    const int count = Listeners.GetSizeI();
    for (int i = 0; i < count; ++i) {
      if (Listeners[i] == listener) {
        Listeners.RemoveAt(i);

        // After removing it from the array, set the dirty flag.
        // Note: Because the flag is atomic, a portable memory fence is implied.
        DirtyListenersCache = 1;

        break;
      }
    }
#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
    if (Listeners.GetSizeI() < 1) {
      ListenersExist.store(false, std::memory_order_relaxed);
    }
#endif
  }
};

//-----------------------------------------------------------------------------
// FloatingCallbackListener
//
// Internal implementation class for the CallbackListener.
// This can only be associated with one CallbackListener object for its lifetime.
template <class DelegateT>
class FloatingCallbackListener : public RefCountBase<FloatingCallbackListener<DelegateT>> {
 public:
  FloatingCallbackListener(DelegateT handler);
  ~FloatingCallbackListener();

  void EnterCancelState();

  bool IsValid() const {
    return Handler.IsValid();
  }

  // TBD: Should these be binned to reduce the lock count?
  // Boost does not do that.  And I am worried about deadlocks when misused.
  mutable Lock ListenerLock;

  // Handler function
  DelegateT Handler;
};

//-----------------------------------------------------------------------------
// Template Implementation: FloatingCallbackEmitter

template <class DelegateT>
bool FloatingCallbackEmitter<DelegateT>::AddListener(
    FloatingCallbackListener<DelegateT>* listener) {
  Lock::Locker locker(GetEmitterLock());

  if (IsShutdown) {
    return false;
  }

  // Add the listener to our list
  Listeners.PushBack(listener);

  // After adding it to the array, set the dirty flag.
  // Note: Because the flag is atomic, a portable memory fence is implied.
  DirtyListenersCache = 1;

#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
  ListenersExist.store(true, std::memory_order_relaxed);
#endif

  return true;
}

// Called from the listener object as it is transitioning to canceled state.
// The listener's mutex is not held during this call.
template <class DelegateT>
void FloatingCallbackEmitter<DelegateT>::OnListenerCancel(
    FloatingCallbackListener<DelegateT>* listener) {
  Lock::Locker locker(GetEmitterLock());

  // If not shut down,
  // Note that if it is shut down then there will be no listeners in the array.
  if (!IsShutdown) {
    // Remove it.
    noLockFindAndRemoveListener(listener);
  }
}

template <class DelegateT>
void FloatingCallbackEmitter<DelegateT>::Shutdown() {
  Lock::Locker locker(GetEmitterLock());

  IsShutdown = true;

  Listeners.ClearAndRelease();

  // Note: Because the flag is atomic, a portable memory fence is implied.
  DirtyListenersCache = 1;

#if !defined(OVR_CC_MSVC) || (OVR_CC_VERSION > 1600) // Newer than VS2010
  ListenersExist.store(false, std::memory_order_relaxed);
#endif
}

//-----------------------------------------------------------------------------
// Call function
//
// (1) Update the cache of listener references, if it has changed.
// (2) For each listener,
//    (a) Hold ListenerLock.
//    (b) If listener handler is valid, call the handler.
#define OVR_EMITTER_CALL_BODY(params)                                               \
  updateListenersCache();                                                           \
  if (IsShutdown)                                                                   \
    return; /* Pure optimization.  It is fine if this races. */                     \
  const int count = ListenersCacheForCalls.GetSizeI();                              \
  for (int i = 0; i < count; ++i) {                                                 \
    Lock::Locker locker(&ListenersCacheForCalls[i]->ListenerLock);                  \
    if (ListenersCacheForCalls[i]->Handler.IsValid()) {                             \
      ListenersCacheForCalls[i]->Handler params; /* Using a macro for this line. */ \
    }                                                                               \
  }

template <class DelegateT>
void FloatingCallbackEmitter<DelegateT>::Call() {
  OVR_EMITTER_CALL_BODY(())
}

template <class DelegateT>
template <class Param1>
void FloatingCallbackEmitter<DelegateT>::Call(Param1* p1) {
  OVR_EMITTER_CALL_BODY((p1))
}

template <class DelegateT>
template <class Param1>
void FloatingCallbackEmitter<DelegateT>::Call(Param1& p1) {
  OVR_EMITTER_CALL_BODY((p1))
}

template <class DelegateT>
template <class Param1, class Param2>
void FloatingCallbackEmitter<DelegateT>::Call(Param1* p1, Param2* p2) {
  OVR_EMITTER_CALL_BODY((p1, p2))
}

template <class DelegateT>
template <class Param1, class Param2>
void FloatingCallbackEmitter<DelegateT>::Call(Param1& p1, Param2& p2) {
  OVR_EMITTER_CALL_BODY((p1, p2))
}

template <class DelegateT>
template <class Param1, class Param2, class Param3>
void FloatingCallbackEmitter<DelegateT>::Call(Param1* p1, Param2* p2, Param3* p3) {
  OVR_EMITTER_CALL_BODY((p1, p2, p3))
}

template <class DelegateT>
template <class Param1, class Param2, class Param3>
void FloatingCallbackEmitter<DelegateT>::Call(Param1& p1, Param2& p2, Param3& p3) {
  OVR_EMITTER_CALL_BODY((p1, p2, p3))
}

#undef OVR_EMITTER_CALL_BODY

//-----------------------------------------------------------------------------
// Template Implementation: FloatingCallbackListener

template <class DelegateT>
FloatingCallbackListener<DelegateT>::FloatingCallbackListener(DelegateT handler)
    : Handler(handler) {
  OVR_ASSERT(Handler.IsValid());
}

template <class DelegateT>
FloatingCallbackListener<DelegateT>::~FloatingCallbackListener() {
  OVR_ASSERT(!Handler.IsValid());
}

template <class DelegateT>
void FloatingCallbackListener<DelegateT>::EnterCancelState() {
  ListenerLock.DoLock();
  Handler.Invalidate();
  ListenerLock.Unlock();
}

} // namespace OVR

#endif // OVR_CallbacksInternal_h
