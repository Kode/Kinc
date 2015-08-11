#pragma once

namespace Kore {
	// Information about the gamepad
	class GamepadInfo {
	public:
		GamepadInfo() : devId(-1), vendor(nullptr), producName(nullptr) {};
		int devId;
		wchar_t* vendor;
		wchar_t* producName;
	};

	class Gamepad {
	public:
		static Gamepad* get(int num);
		void (*Axis)(int, float);
		void (*Button)(int, float);

		//called by backend
		void _axis(int axis, float value);
		void _button(int button, float value);

		GamepadInfo info;
	};
}
