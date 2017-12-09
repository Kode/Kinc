#pragma once

#include <linux/joystick.h>
#include <string>

namespace Kore {

	class HIDGamepad {

	public:
		HIDGamepad(int index);
		~HIDGamepad();
		void Update();

	private:
		int idx;
		std::string gamepad_dev_name;
		int file_descriptor;
		int connected;
		struct js_event gamepadEvent;

		int Open();
		int Close();
		void ProcessEvent(struct js_event e);
	};
} // namespace Kore
