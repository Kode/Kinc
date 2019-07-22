#pragma once

namespace Kore {
	class Gamepad {
	public:
		static Gamepad* get(int num);
		void (*Axis)(int, float);
		void (*Button)(int, float);

		const char* vendor;
		const char* productName;

		bool connected();

		Gamepad() : Axis(nullptr), Button(nullptr), vendor(nullptr), productName(nullptr) {}
	private:
		int num;
	};
}
