#include "pch.h"

#include <kinc/display.h>
#include <kinc/log.h>

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

int kinc_display_count_available_modes(int display) {
	return 1;
}

bool kinc_display_available(int display) {
	return true;
}

const char *kinc_display_name(int display) {
	return "Display";
}

kinc_display_mode_t kinc_display_current_mode(int display) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

int kinc_count_displays() {
	return 1;
}

int kinc_primary_display() {
	return 0;
}
