#pragma once

/*
#ifdef KORE_LINUX
int number;
#endif

#ifdef KORE_LINUX
number = -1;
#endif
*/

namespace Kore {
	struct DisplayMode {
		int width;
		int height;
		int frequency;
	};

	class Display {
	public:
	    bool available;
	    char name[32];
	    int x;
	    int y;
	    int width;
	    int height;
		int frequency;
		int pixelsPerInch;

	    Display() : available(false), pixelsPerInch(72) {
			name[0] = 0;
	    }
				
		static Display* primary();
		static Display* get(int index);
		static int count();

		DisplayMode availableMode(int index);
		int countAvailableModes();
		void setMode(int index);
	};
}
