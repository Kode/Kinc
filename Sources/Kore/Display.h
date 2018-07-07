#pragma once

#include <Kore/DisplayData.h>

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
		int bitsPerPixel;
	};

	class Display {
	public:
		static Display* primary();
		static Display* get(int index);
		static int count();

	    bool available();
	    const char* name();
	    int x();
	    int y();
	    int width();
	    int height();
		int frequency();
		int pixelsPerInch();
		
		DisplayMode availableMode(int index);
		int countAvailableModes();

		DisplayData _data;
		Display();
	};
}
