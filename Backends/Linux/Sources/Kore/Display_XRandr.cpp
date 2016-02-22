#if defined(KORE_LINUX_SYSTEM_SCREENS_XRANDR)

#include "pch.h"
#include <Kore/Log.h>

#include <X11/extensions/Xrandr.h>

namespace Kore { namespace System { namespace Monitor {

void enumDisplayMonitors( Screen screens[], int & screenCounter ) {
    Display * dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        fatalError("could not open display");
    }

    int major;
    int minor;

    if (XRRQueryVersion(dpy, major, minor())) {
    }
}

}}}

#endif #if defined(KORE_LINUX_SYSTEM_SCREENS_XRANDR)
