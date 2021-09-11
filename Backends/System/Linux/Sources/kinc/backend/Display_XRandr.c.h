#if defined(KORE_LINUX_SYSTEM_SCREENS_XRANDR)

#include <X11/extensions/Xrandr.h>

void enumDisplayMonitors(kinc_display_t *displays, int *displayCounter) {
	Display *dpy = XOpenDisplay(NULL);

	if (dpy == NULL) {
		fatalError("could not open display");
	}

	int major;
	int minor;

	if (XRRQueryVersion(dpy, major, minor())) {
	}
}

#endif
