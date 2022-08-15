#include <kinc/display.h>
#include <kinc/log.h>

void print_mode(const char *indent, kinc_display_mode_t mode) {
	kinc_log(KINC_LOG_LEVEL_INFO, "%sx: %i", indent, mode.x);
	kinc_log(KINC_LOG_LEVEL_INFO, "%sy: %i", indent, mode.y);
	kinc_log(KINC_LOG_LEVEL_INFO, "%swidth: %i", indent, mode.width);
	kinc_log(KINC_LOG_LEVEL_INFO, "%sheight: %i", indent, mode.height);
	kinc_log(KINC_LOG_LEVEL_INFO, "%spixels_per_inch: %i", indent, mode.pixels_per_inch);
	kinc_log(KINC_LOG_LEVEL_INFO, "%sfrequency: %i", indent, mode.frequency);
	kinc_log(KINC_LOG_LEVEL_INFO, "%sbits_per_pixel: %i", indent, mode.bits_per_pixel);
}

int kickstart(int argc, char **argv) {
	bool print_modes = false;
	if (argc > 1 && strcmp(argv[1], "--print-modes") == 0) {
		print_modes = true;
	}
	kinc_display_init();
	int count = kinc_count_displays();
	kinc_log(KINC_LOG_LEVEL_INFO, "display count: %i", count);
	kinc_log(KINC_LOG_LEVEL_INFO, "primary display: %i", kinc_primary_display());
	for (int i = 0; i < count; i++) {
		bool available = kinc_display_available(i);
		kinc_log(KINC_LOG_LEVEL_INFO, "display %i:", i);
		kinc_log(KINC_LOG_LEVEL_INFO, "\tavailable: %s", available ? "true" : "false");
		if (available) {
			kinc_log(KINC_LOG_LEVEL_INFO, "\tname: %s", kinc_display_name(i));
			kinc_display_mode_t mode = kinc_display_current_mode(i);
			print_mode("\t", mode);
			kinc_log(KINC_LOG_LEVEL_INFO, "");
			int mode_count = kinc_display_count_available_modes(i);
			kinc_log(KINC_LOG_LEVEL_INFO, "\tavailable modes: %i", mode_count);
			if (print_modes) {
				for (int j = 0; j < mode_count; j++) {
					kinc_display_mode_t mode = kinc_display_available_mode(i, j);
					print_mode("\t\t", mode);
					kinc_log(KINC_LOG_LEVEL_INFO, "");
				}
			}
		}
	}
	return 0;
}