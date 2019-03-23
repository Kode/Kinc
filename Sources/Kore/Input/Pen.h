#pragma once

namespace Kore {
	class Pen {
	public:
		static Pen* the();
		void (*Move)(int windowId, int x, int y, float pressure);
		void (*Press)(int windowId, int x, int y, float pressure);
		void (*Release)(int windowId, int x, int y, float pressure);
		Pen();
	};
}
