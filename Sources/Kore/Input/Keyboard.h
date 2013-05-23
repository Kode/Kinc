#pragma once

namespace Kore {
	class KeyEvent;

	class Keyboard {
	public:
		static Keyboard* the();
		void (*KeyDown)(KeyEvent*);
		void (*KeyUp)(KeyEvent*);
		void clear();

		//called by backend
		void keydown(KeyEvent);
		void keyup(KeyEvent);
	};
}
