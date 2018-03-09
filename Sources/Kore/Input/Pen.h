#pragma once

namespace Kore {
	class Pen {
	public:
		Pen();
		static Pen* the();
		void (*Move)(int windowId, int x, int y, float pressure);
		void (*Press)(int windowId, int x, int y, float pressure);
		void (*Release)(int windowId, int x, int y, float pressure);

		// For backend
		void _move(int windowId, int x, int y, float pressure);
		void _press(int windowId, int x, int y, float pressure);
		void _release(int windowId, int x, int y, float pressure);
	};
}
