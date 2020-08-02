/************************************************************************************

Filename    :   Util_LongPollThread.cpp
Content     :   Allows us to do all long polling tasks from a single thread to minimize deadlock
risk
Created     :   June 30, 2013
Authors     :   Chris Taylor

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

#include "Util_LongPollThread.h"
#include "Util_Watchdog.h"

OVR_DEFINE_SINGLETON(OVR::Util::LongPollThread);

namespace OVR {
namespace Util {

void LongPollThread::AddPollFunc(CallbackListener<PollFunc>* func) {
  PollSubject.AddListener(func);
}

LongPollThread::LongPollThread() : Terminated(false) {
  LongPollThreadHandle = std::make_unique<std::thread>([this] { this->Run(); });

  // Must be at end of function
  PushDestroyCallbacks();
}

LongPollThread::~LongPollThread() {
  OVR_ASSERT(!LongPollThreadHandle->joinable());
}

void LongPollThread::OnThreadDestroy() {
  fireTermination();

  LongPollThreadHandle->join();
}

void LongPollThread::Wake() {
  WakeEvent.SetEvent();
}

void LongPollThread::fireTermination() {
  Terminated.store(true, std::memory_order_relaxed);
  Wake();
}

void LongPollThread::OnSystemDestroy() {
  delete this;
}

void LongPollThread::Run() {
  Thread::SetCurrentThreadName("LongPoll");
  WatchDog watchdog("LongPoll");

  // While not terminated,
  do {
    watchdog.Feed(10000);

    PollSubject.Call();

    WakeEvent.Wait(WakeupInterval);
    WakeEvent.ResetEvent();
  } while (!Terminated.load(std::memory_order_acquire));
}

} // namespace Util
} // namespace OVR
