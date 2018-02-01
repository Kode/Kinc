/**************************************************************************

Filename    :   OVR_Rand.cpp
Content     :   Random number generator
Created     :   Aug 28, 2014
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

#include "OVR_Rand.h"
#include "OVR_Timer.h"

namespace OVR {

void RandomNumberGenerator::SeedRandom() {
  uint64_t seed = Timer::GetTicksNanos();

  uint32_t x = (uint32_t)seed, y = (uint32_t)(seed >> 32);

  Seed(x, y);
}

void RandomNumberGenerator::Seed(uint32_t x, uint32_t y) {
  // Based on the mixing functions of MurmurHash3
  static const uint64_t C1 = 0xff51afd7ed558ccdULL;
  static const uint64_t C2 = 0xc4ceb9fe1a85ec53ULL;

  x += y;
  y += x;

  uint64_t seed_x = 0x9368e53c2f6af274ULL ^ x;
  uint64_t seed_y = 0x586dcd208f7cd3fdULL ^ y;

  seed_x *= C1;
  seed_x ^= seed_x >> 33;
  seed_x *= C2;
  seed_x ^= seed_x >> 33;

  seed_y *= C1;
  seed_y ^= seed_y >> 33;
  seed_y *= C2;
  seed_y ^= seed_y >> 33;

  Rx = seed_x;
  Ry = seed_y;

  // Inlined Next(): Discard first output

  Rx = (uint64_t)0xfffd21a7 * (uint32_t)Rx + (uint32_t)(Rx >> 32);
  Ry = (uint64_t)0xfffd1361 * (uint32_t)Ry + (uint32_t)(Ry >> 32);

  // Throw away any saved RandN() state
  HaveRandN = false;

  Seeded = true;
}

} // namespace OVR
