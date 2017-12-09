#pragma once

#include <linux/joystick.h>

namespace Kore {

	class HIDGamepad {

	public:
		HIDGamepad(int index);
		~HIDGamepad();
		void Update();

	private:
		int idx;
		char gamepad_dev_name[256];
		int file_descriptor;
		int connected;
		struct js_event gamepadEvent;

		int Open();
		int Close();
		void ProcessEvent(struct js_event e);
	};
} // namespace Kore
