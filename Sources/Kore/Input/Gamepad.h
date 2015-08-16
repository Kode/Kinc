#pragma once

namespace Kore {
	class Gamepad {
	public:
		static Gamepad* get(int num);
		void (*Axis)(int, float);
		void (*Button)(int, float);

		//called by backend
		void _axis(int axis, float value);
		void _button(int button, float value);

		const char* vendor;
		const char* productName;

		Gamepad() : vendor(nullptr), productName(nullptr) { }
	};
}
