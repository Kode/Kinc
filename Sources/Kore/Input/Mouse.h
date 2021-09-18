#pragma once

namespace Kore {
	class Mouse {
	public:
		static Mouse *the();

		void (*Move)(int windowId, int x, int y, int movementX, int movementY);
		void (*Press)(int windowId, int button, int x, int y);
		void (*Release)(int windowId, int button, int x, int y);
		void (*Scroll)(int windowId, int delta);
		void (*Leave)(int windowId);

		bool canLock();
		bool isLocked();
		void lock(int window);
		void unlock();

		void show(bool truth);
		void setCursor(int cursor);

		void setPosition(int windowId, int x, int y);
		void getPosition(int windowId, int &x, int &y);

		Mouse();
	};
}
