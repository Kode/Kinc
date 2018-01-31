/************************************************************************************

PublicHeader:   Kernel
Filename    :   OVR_Callbacks.h
Content     :   Callback library
Created     :   June 20, 2014
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

#ifndef OVR_Callbacks_h
#define OVR_Callbacks_h

#include "OVR_CallbacksInternal.h"

#include "OVR_String.h" // For CallbackHash
#include "OVR_Hash.h" // For CallbackHash

namespace OVR {

//-----------------------------------------------------------------------------
// CallbackEmitter
//
// Emitter of callbacks.
// Thread-safety: All public members may be safely called concurrently.
template <class DelegateT>
class CallbackEmitter : public NewOverrideBase {
 public:
  CallbackEmitter();
  ~CallbackEmitter();

  // Add a listener.
  bool AddListener(CallbackListener<DelegateT>* listener);

  // Get the current number of listeners.  Note that this can change as other threads
  // add listeners to the emitter.
  int GetListenerCount() const;

  bool HasListeners() const {
    return Emitter->HasListeners();
  }

  void Call() {
    Emitter->Call();
  }
  template <class Param1>
  void Call(Param1* p1) {
    Emitter->Call(p1);
  }
  template <class Param1>
  void Call(Param1& p1) {
    Emitter->Call(p1);
  }
  template <class Param1, class Param2>
  void Call(Param1* p1, Param2* p2) {
    Emitter->Call(p1, p2);
  }
  template <class Param1, class Param2>
  void Call(Param1& p1, Param2& p2) {
    Emitter->Call(p1, p2);
  }
  template <class Param1, class Param2, class Param3>
  void Call(Param1* p1, Param2* p2, Param3* p3) {
    Emitter->Call(p1, p2, p3);
  }
  template <class Param1, class Param2, class Param3>
  void Call(Param1& p1, Param2& p2, Param3& p3) {
    Emitter->Call(p1, p2, p3);
  }

  // Remove all listeners and prevent further listeners from being added.
  void Shutdown();

 protected:
  Ptr<FloatingCallbackEmitter<DelegateT>> Emitter;
};

//-----------------------------------------------------------------------------
// CallbackListener
//
// Listener for callbacks.
// Thread-safety: Operations on a listener are not thread-safe.
// The listener may only listen to *one emitter* at a time.
template <class DelegateT>
class CallbackListener : public NewOverrideBase {
  friend class CallbackEmitter<DelegateT>;

 public:
  CallbackListener();
  ~CallbackListener();

  // Stop listening to callbacks.
  // And set a new handler for callbacks.
  void SetHandler(DelegateT handler);

  // Is listening to an emitter at this instant?
  // If the Emitter has shutdown, then this may inaccurately return true.
  bool IsListening() const;

  // Stops listening to callbacks.
  void Cancel();

 protected:
  /// Internal data:

  // Reference to the associated listener.
  Ptr<FloatingCallbackListener<DelegateT>> FloatingListener;

  // Reference to the associated emitter.
  Ptr<FloatingCallbackEmitter<DelegateT>> FloatingEmitter;

  DelegateT Handler;
};

//-----------------------------------------------------------------------------
// Template Implementation: CallbackEmitter

template <class DelegateT>
CallbackEmitter<DelegateT>::CallbackEmitter() {
  Emitter = *new FloatingCallbackEmitter<DelegateT>;
}

template <class DelegateT>
CallbackEmitter<DelegateT>::~CallbackEmitter() {
  Emitter->Shutdown();
  // Emitter goes out of scope here.
}

template <class DelegateT>
bool CallbackEmitter<DelegateT>::AddListener(CallbackListener<DelegateT>* listener) {
  // The listener object can only be attached to one emitter at a time.
  // The caller should explicitly Cancel() a listener before listening
  // to a new emitter, even if it is the same emitter.
  OVR_ASSERT(!listener->FloatingEmitter && !listener->FloatingListener);

  if (listener->FloatingEmitter || listener->FloatingListener) {
    // Cancel any previous listening
    listener->Cancel();
  }

  // Set the floating listener and emitter
  listener->FloatingListener = *new FloatingCallbackListener<DelegateT>(listener->Handler);
  listener->FloatingEmitter = Emitter.GetPtr();

  // The remaining input checks are performed inside.
  return Emitter->AddListener(listener->FloatingListener);
}

template <class DelegateT>
int CallbackEmitter<DelegateT>::GetListenerCount() const {
  return Emitter->Listeners.GetSizeI();
}

template <class DelegateT>
void CallbackEmitter<DelegateT>::Shutdown() {
  Emitter->Shutdown();
}

//-----------------------------------------------------------------------------
// Template Implementation: CallbackListener

template <class DelegateT>
CallbackListener<DelegateT>::CallbackListener() {
  // Listener is null until a handler is set.
}

template <class DelegateT>
CallbackListener<DelegateT>::~CallbackListener() {
  Cancel();
}

template <class DelegateT>
void CallbackListener<DelegateT>::Cancel() {
  if (FloatingListener) {
    FloatingListener->EnterCancelState();
  }

  if (FloatingEmitter) {
    if (FloatingListener) {
      FloatingEmitter->OnListenerCancel(FloatingListener);
    }
  }

  // FloatingEmitter goes out of scope here.
  FloatingEmitter = nullptr;

  // FloatingListener goes out of scope here.
  FloatingListener = nullptr;
}

template <class DelegateT>
void CallbackListener<DelegateT>::SetHandler(DelegateT handler) {
  Cancel();

  Handler = handler;
}

template <class DelegateT>
bool CallbackListener<DelegateT>::IsListening() const {
  if (!FloatingListener.GetPtr()) {
    return false;
  }

  return FloatingListener->IsValid();
}

//-----------------------------------------------------------------------------
// CallbackHash
//
// A hash containing CallbackEmitters
template <class DelegateT>
class CallbackHash : public NewOverrideBase {
  typedef Hash<String, CallbackEmitter<DelegateT>*, String::HashFunctor> HashTable;

 public:
  ~CallbackHash() {
    Clear();
  }

  void Clear() {
    for (auto ii = Table.Begin(); ii != Table.End(); ++ii) {
      delete ii->Second;
    }

    Table.Clear();
  }

  CallbackEmitter<DelegateT>* GetKey(String key) {
    CallbackEmitter<DelegateT>** emitter = Table.Get(key);
    if (emitter) {
      return *emitter;
    }
    return nullptr;
  }

  void AddListener(String key, CallbackListener<DelegateT>* listener) {
    CallbackEmitter<DelegateT>** pEmitter = Table.Get(key);
    CallbackEmitter<DelegateT>* emitter = nullptr;

    if (!pEmitter) {
      emitter = new CallbackEmitter<DelegateT>;
      Table.Add(key, emitter);
    } else {
      emitter = *pEmitter;
    }

    emitter->AddListener(listener);
  }

  void RemoveKey(String key) {
    CallbackEmitter<DelegateT>** emitter = Table.Get(key);

    if (emitter) {
      delete *emitter;
      Table.Remove(key);
    }
  }

 protected:
  HashTable Table; // Hash table
};

} // namespace OVR

#endif // OVR_Callbacks_h
