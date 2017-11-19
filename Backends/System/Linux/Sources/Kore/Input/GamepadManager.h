// Modified version of the joystick/gamepad implemenation found in SFML
// https://github.com/SFML/SFML/blob/master/src/SFML/Window/Unix/JoystickImpl.cpp

#ifndef GAMEPAD_MANAGER_H
#define GAMEPAD_MANAGER_H

#include <array>
#include <linux/input.h>
#include <vector>
#include <string>

namespace Kore {

	class GamepadManager {
	public:
		GamepadManager();
		~GamepadManager();
		void update();

	private:
		void initialize();
		bool isGamepadConnected(unsigned int index);
		void terminate();
		int gamePads;
		std::vector<struct LinuxGamepad> gamepadsVector;
	};

	struct LinuxGamepad {
		LinuxGamepad(unsigned int index) {
			setInitState(index);
		}

		void setInitState(unsigned int index);
		bool open();
		bool update();
		void close();

		unsigned int index;  // Gamepad index
		bool connected;      // Is the Gamepad currently connected?
		int file_descriptor; // File descriptor of the Gamepad
		std::string name;    // Name of the Gamepad
	};

} // namespace Kore

#endif // GAMEPAD_MANAGER_H
