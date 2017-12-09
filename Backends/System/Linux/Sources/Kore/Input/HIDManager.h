#pragma once

#include "HIDGamepad.h"
#include <vector>

namespace Kore {

	class HIDManager {

	public:
		HIDManager();
		~HIDManager();
		void Update();

	private:
		int gamepads;
		std::vector<HIDGamepad*>* gamepadList;
	};
} // namespace Kore
