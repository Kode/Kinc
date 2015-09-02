#pragma once

namespace Kore {
	class Mouse {
	public:
		static Mouse* the();
		void (*Move)(int x, int y);
		void (*Press)(int button, int x, int y);
		void (*Release)(int button, int x, int y);
		void (*Scroll)(int delta);
		
		//for backend
		void _move(int x, int y);
		void _press(int button, int x, int y);
		void _release(int button, int x, int y);
		void _scroll(int delta);
	};
}
