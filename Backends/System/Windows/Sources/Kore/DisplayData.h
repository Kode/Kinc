#pragma once

struct HMONITOR__;

namespace Kore {
	struct DisplayData {
		HMONITOR__* id;
		char name[32];
		bool primary, available, modeChanged;
		int index, x, y, width, height, ppi, frequency;
		DisplayData();
	};
}
