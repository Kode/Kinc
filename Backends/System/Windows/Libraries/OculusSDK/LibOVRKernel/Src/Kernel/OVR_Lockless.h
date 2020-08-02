/************************************************************************************

Filename    :   OVR_Lockless.h
Content     :   Lock-less classes for producer/consumer communication
Created     :   November 9, 2013
Authors     :   John Carmack

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

#ifndef OVR_Lockless_h
#define OVR_Lockless_h

#include <cstring>
using std::memcpy;

#include "OVR_Atomic.h"

// Define this to compile-in Lockless test logic
//#define OVR_LOCKLESS_TEST

namespace OVR {

// ***** LocklessUpdater

// For single producer cases where you only care about the most recent update, not
// necessarily getting every one that happens (vsync timing, SensorFusion updates).
//
// This is multiple consumer safe, but is currently only used with a single consumer.
//
// The SlotType can be the same as T, but should probably be a larger fixed size.
// This allows for forward compatibility when the updater is shared between processes.

template <class T, class SlotType = T>
class LocklessUpdater {
 public:
  LocklessUpdater() {
    OVR_COMPILER_ASSERT(sizeof(T) <= sizeof(SlotType));
  }

  T GetState() const {
    // Copy the state out, then retry with the alternate slot
    // if we determine that our copy may have been partially
    // stepped on by a new update.
    T state;
    int begin, end, final;

    for (;;) {
      // We are adding 0, only using these as atomic memory barriers, so it
      // is ok to cast off the const, allowing GetState() to remain const.
      end = UpdateEnd.load(std::memory_order_acquire);
      state = Slots[end & 1];
      begin = UpdateBegin.load(std::memory_order_acquire);
      if (begin == end) {
        break;
      }

      // The producer is potentially blocked while only having partially
      // written the update, so copy out the other slot.
      state = Slots[(begin & 1) ^ 1];
      final = UpdateBegin.load(std::memory_order_acquire);
      if (final == begin) {
        break;
      }

      // The producer completed the last update and started a new one before
      // we got it copied out, so try fetching the current buffer again.
    }
    return state;
  }

  void SetState(const T& state) {
    const int slot = UpdateBegin.fetch_add(1) & 1;
    // Write to (slot ^ 1) because ExchangeAdd returns 'previous' value before add.
    Slots[slot ^ 1] = state;
    UpdateEnd.fetch_add(1);
  }

  std::atomic<int> UpdateBegin = {0};
  std::atomic<int> UpdateEnd = {0};
  SlotType Slots[2];
};

#pragma pack(push, 8)

// Padded out version stored in the updater slots
// Designed to be a larger fixed size to allow the data to grow in the future
// without breaking older compiled code.
OVR_DISABLE_MSVC_WARNING(4351)
template <class Payload, int PaddingSize>
struct LocklessPadding {
  uint8_t buffer[PaddingSize];

  LocklessPadding() : buffer() {}

  LocklessPadding& operator=(const Payload& rhs) {
    // if this fires off, then increase PaddingSize
    // IMPORTANT: this WILL break backwards compatibility
    static_assert(sizeof(buffer) >= sizeof(Payload), "PaddingSize is too small");

    memcpy(buffer, &rhs, sizeof(Payload));
    return *this;
  }

  operator Payload() const {
    Payload result;
    memcpy(&result, buffer, sizeof(Payload));
    return result;
  }
};
OVR_RESTORE_MSVC_WARNING()

#pragma pack(pop)

// FIXME: Move this somewhere else

// ***** LocklessBuffer

// FIXME: update these comments
// For single producer cases where you only care about the most recent update, not
// necessarily getting every one that happens (vsync timing, SensorFusion updates, external camera
// frames).
//
// The writer writes an incrementing generation # for each write start, and write end
// The reader reads the last written generation number, saves it, does its operations, then
// reads the latest value of the last written generation number.  If they match, there was no
// collision, and the work is done.  If not, the reader has to loop until it gets a matching
// Last written generation number
//
// This is to update & read a dynamically sized object in shared memory.
// Initial use case is for frame buffers for cameras, which are an unknown size until runtime

#pragma pack(push, 1)

class LocklessBuffer {
 public:
  LocklessBuffer() {
    ;
  }

  void Initialize(unsigned bufferSize) {
    BufferSize = bufferSize;
    LastWrittenGeneration = 0;
  }

  char* StartWrite(unsigned offset) {
    ++LastWrittenGeneration;
    return GetDataForWrite(offset);
  }

  unsigned GetBufferSize() const {
    return BufferSize;
  }

  int EndWrite() {
    return ++LastWrittenGeneration;
  }

  int GetLastWrittenGeneration() const {
    return LastWrittenGeneration;
  }

  const char* GetDataForRead(unsigned offset = 0) const {
    return &(Data[offset]);
  }
  char* GetDataForWrite(unsigned offset = 0) {
    return &(Data[offset]);
  }

  bool DidReadCollide(int lastReadGeneration) const {
    return lastReadGeneration != LastWrittenGeneration;
  }

 private:
  unsigned BufferSize = 0;
  std::atomic<int> LastWrittenGeneration;

  // Data starts here...
  char Data[1];
};

#pragma pack(pop)

} // namespace OVR

#endif // OVR_Lockless_h
