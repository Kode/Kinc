#pragma once

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

		int _index;
		Display();
	};
}
