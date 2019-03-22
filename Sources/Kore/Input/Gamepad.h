#pragma once

namespace Kore {
	class Gamepad {
	public:
		static Gamepad* get(int num);
		void (*Axis)(int, float);
		void (*Button)(int, float);

		const char* vendor;
		const char* productName;

		Gamepad() : vendor(nullptr), productName(nullptr) {}
	};
}
