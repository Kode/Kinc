/************************************************************************************

PublicHeader:   OVR
Filename    :   OVR_System.h
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

#ifndef OVR_System_h
#define OVR_System_h

#include "OVR_Allocator.h"
#include "OVR_Atomic.h"

namespace OVR {

//-----------------------------------------------------------------------------
// SystemSingleton

// Subsystems are implemented using the Singleton pattern.
// To avoid code duplication in all the places where Singletons are defined,
// The pattern is defined once here and used everywhere.

class SystemSingletonInternal {
  friend class System;

  // Allows for including this class in the shutdown list.
  SystemSingletonInternal* NextShutdownSingleton;

  // No copying allowed
  OVR_NON_COPYABLE(SystemSingletonInternal);

 public:
  // Call this to register for a call to OnThreadDestroy and OnSystemDestroy before the
  // Kernel is shut down. OnThreadDestroy is called before any remaining existing threads
  // are exited. Registered callbacks are called in the reverse order they were registered.
  // You would typically call this at the end of your SystemSingletonInternal subclass' constructor.
  // The registered objects are not deleted on shutdown; they would have to do that themselves
  // within
  // OnSystemDestroy or via a compiler-generated static destruction.
  void RegisterDestroyCallback();
  void PushDestroyCallbacks() {
    RegisterDestroyCallback();
  } // For backward compatibility.

 protected:
  SystemSingletonInternal() : NextShutdownSingleton(nullptr) {}

  virtual ~SystemSingletonInternal() {}

  // Initializes the SystemSingletonInternal.
  // You can register for an automatic call to this function by calling PushInitCallbacks.
  // The registration of an automatic call to this is not required, but may be useful if
  // you need to postpone your initialization until after Kernel is initialized.
  // You cannot call PushInitCallbacks or PushDestroyCallbacks while within this function.
  virtual void OnSystemInit() {}

  // Called just before waiting for threads to exit.
  // Listeners are called in the opposite order they were registered.
  // This function is useful for terminating threads at the right time before the rest of the system
  // is shut down.
  // Note: The singleton must not delete itself here, as OnSystemDestroy will subsequently be called
  // for it.
  virtual void OnThreadDestroy() {}

  // Shuts down the SystemSingletonInternal.
  // You can register for an automatic call to this function by calling PushDestroyCallbacks.
  // The registration of an automatic call to this is not required, but may be useful if
  // you need to delay your shutdown until application exit time.
  // You cannot call PushInitCallbacks or PushDestroyCallbacks while within this function.
  // This function may delete this.
  virtual void OnSystemDestroy() {}
};

// Singletons derive from this class
template <class T>
class SystemSingletonBase : public SystemSingletonInternal {
  static std::atomic<T*> SingletonInstance;
  static T* SlowGetInstance();

  struct ZeroInitializer {
    ZeroInitializer() {
      SingletonInstance = nullptr;
    }
  };
  ZeroInitializer zeroInitializer;

 protected:
  ~SystemSingletonBase() {
    // Make sure the instance gets set to zero on dtor
    if (SingletonInstance.load() == this)
      SingletonInstance = nullptr;
  }

 public:
  static OVR_FORCE_INLINE T* GetInstance() {
    // Fast version
    // Note: The singleton instance is stored in an std::atomic<> to allow it to be accessed
    // atomically from multiple threads without locks.
    T* instance = SingletonInstance;
    return instance ? instance : SlowGetInstance();
  }
};

// For reference, see N3337 14.5.1.3 (Static data members of class templates):
template <class T>
std::atomic<T*> OVR::SystemSingletonBase<T>::SingletonInstance;

// Place this in the singleton class in the header file
#define OVR_DECLARE_SINGLETON(T)            \
  friend class OVR::SystemSingletonBase<T>; \
                                            \
 private:                                   \
  T();                                      \
  virtual ~T();                             \
  virtual void OnSystemDestroy() override;

// Place this in the singleton class source file
#define OVR_DEFINE_SINGLETON(T)                  \
  namespace OVR {                                \
  template <>                                    \
  T* SystemSingletonBase<T>::SlowGetInstance() { \
    static OVR::Lock lock;                       \
    OVR::Lock::Locker locker(&lock);             \
    if (!SingletonInstance.load())               \
      SingletonInstance = new T;                 \
    return SingletonInstance;                    \
  }                                              \
  }

// ***** System Core Initialization class

// System initialization must take place before any other OVR_Kernel objects are used;
// this is done my calling System::Init(). Among other things, this is necessary to
// initialize the memory allocator. Similarly, System::Destroy must be
// called before program exist for proper cleanup. Both of these tasks can be achieved by
// simply creating System object first, allowing its constructor/destructor do the work.

class System {
 public:
  // Returns 'true' if system was properly initialized.
  static bool OVR_CDECL IsInitialized();

  // Initializes System core.  Users can override memory implementation by passing
  // a different Allocator here.
  static void OVR_CDECL Init();

  // Halt all system threads (call before Destroy).
  static void OVR_CDECL Stop();

  // De-initializes System more, finalizing the threading system and destroying
  // the global memory allocator.
  static void OVR_CDECL Destroy();

  // Dump any leaked allocations
  static void OVR_CDECL CheckForAllocatorLeaks();
};

} // namespace OVR

#endif
