#include "pch.h"

#include "../Sources/C/Kore/Display.h"

int Kore_PrimaryDisplay() {
	return 0;
}

int Kore_CountDisplays() {
	return 1;
}

bool Kore_DisplayAvailable(int display_index) {
	return display_index == 0;
}

const char *Kore_DisplayName(int display_index) {
	return "Display";
}

Kore_DisplayMode Kore_DisplayCurrentMode(int display_index) {
	Kore_DisplayMode mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}

int Kore_DisplayCountAvailableModes(int display_index) {
	return 1;
}

Kore_DisplayMode Kore_DisplayAvailableMode(int display_index, int mode_index) {
	return Kore_DisplayCurrentMode(0);
}
