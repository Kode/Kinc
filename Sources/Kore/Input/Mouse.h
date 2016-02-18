#pragma once

namespace Kore {
	class Mouse {
	public:
		Mouse();
		static Mouse* the();
		void (*Move)(int windowId, int x, int y, int movementX, int movementY);
		void (*Press)(int windowId, int button, int x, int y);
		void (*Release)(int windowId, int button, int x, int y);
		void (*Scroll)(int windowId, int delta);
		
		bool canLock(int windowId);
		bool isLocked(int windowId);
		void lock(int windowId);
		void unlock(int windowId);
		
		void show(bool truth);
		void setPosition(int windowId, int x, int y);
		void getPosition(int windowId, int& x, int& y);

		//for backend
		void _move(int windowId, int x, int y);
		void _press(int windowId, int button, int x, int y);
		void _release(int windowId, int button, int x, int y);
		void _scroll(int windowId, int delta);
		void _activated(int windowId, bool truth);

	private:
		void _lock(int windowId, bool truth);

	private:
		// TODO (DK) states must be saved for every window?
		int lastX;
		int lastY;
		int lockX;
		int lockY;
		int centerX;
		int centerY;
		bool locked;
		bool moved;
	};
}
