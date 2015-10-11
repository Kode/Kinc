#pragma once

namespace Kore {
	class Mouse {
	public:
		Mouse();
		static Mouse* the();
		void (*Move)(int x, int y, int movementX, int movementY);
		void (*Press)(int button, int x, int y);
		void (*Release)(int button, int x, int y);
		void (*Scroll)(int delta);
		
		bool canLock();
		bool isLocked();
		void lock();
		void unlock();
		
		void show(bool truth);
		void setPosition(int x, int y);
		void getPosition(int& x, int& y);

		//for backend
		void _move(int x, int y);
		void _press(int button, int x, int y);
		void _release(int button, int x, int y);
		void _scroll(int delta);
		void _activated(bool truth);

	private:
		void _lock(bool truth);

	private:
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
