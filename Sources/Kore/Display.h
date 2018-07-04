#pragma once

/*
#ifdef KORE_WINDOWS
struct HMONITOR__;
typedef HMONITOR__* HMONITOR;
#endif

#ifdef KORE_WINDOWS
HMONITOR id;
#endif
#ifdef KORE_LINUX
int number;
#endif

#ifdef KORE_WINDOWS
id = nullptr;
#endif
#ifdef KORE_LINUX
number = -1;
#endif
*/

namespace Kore {
	class Display {
	    bool available;
	    char name[32];
	    int x;
	    int y;
	    int width;
	    int height;
	    bool primary;
		int pixelsPerInch;

	    Display() {
			available = false;
			name[0] = 0;
	        primary = false;
			pixelsPerInch = 72;
	    }

		void enumerate();
		static Display* primary();
		static Display* get(int index);
		static int count();
	};
}
