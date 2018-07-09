#pragma once

struct HMONITOR__;

namespace Kore {
	class Display;

	namespace Windows {
		void initDisplays();
		Display* getDisplayForMonitor(HMONITOR__* monitor);
		void restoreDisplay(int display);
		void restoreDisplays();
		void hideWindows();
		void destroyWindows();
	}
}
