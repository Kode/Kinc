/************************************************************************************

Filename    :   Util_LongPollThread.h
Content     :   Allows us to do all long polling tasks from a single thread to minimize deadlock
risk Created     :   June 30, 2013 Authors     :   Chris Taylor

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

*************************************************************************************/

#ifndef OVR_Util_LongPollThread_h
#define OVR_Util_LongPollThread_h

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_Allocator.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Threads.h"
#include "Kernel/OVR_Callbacks.h"
#include <thread>

namespace OVR {
namespace Util {

//-----------------------------------------------------------------------------
// LongPollThread

// This thread runs long-polling subsystems that wake up every second or so
// The motivation is to reduce the number of threads that are running to minimize the risk of
// deadlock
class LongPollThread : public SystemSingletonBase<LongPollThread> {
  OVR_DECLARE_SINGLETON(LongPollThread);
  virtual void OnThreadDestroy() override;

 public:
  typedef Delegate0<void> PollFunc;
  static const int WakeupInterval = 1000; // milliseconds

  void AddPollFunc(CallbackListener<PollFunc>* func);

  void Wake();

  // debug method for assertion to maintain initialization order for this singleton
  static bool IsInitialized();

 protected:
  CallbackEmitter<PollFunc> PollSubject;

  std::atomic<bool> Terminated;
  Event WakeEvent;
  std::unique_ptr<std::thread> LongPollThreadHandle;

  void fireTermination();

  void Run();
};
} // namespace Util
} // namespace OVR

#endif // OVR_Util_LongPollThread_h
