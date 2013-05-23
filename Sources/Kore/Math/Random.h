#pragma once

namespace Kore {
	namespace Random {
		void init(int seed);
		s32 get();
		s32 get(s32 max);
		s32 get(s32 min, s32 max);
	}
}
