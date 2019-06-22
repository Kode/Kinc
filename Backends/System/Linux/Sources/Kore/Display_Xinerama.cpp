#include "pch.h"

#include <kinc/display.h>

#include <stdlib.h>

//#define KORE_LINUX_DISPLAY_XINERAMA 1

//#if defined(KORE_LINUX_DISPLAY_XINERAMA)

#include <Kore/Display.h>
#include <Kore/Log.h>

#ifdef KORE_OPENGL
#include <X11/X.h>
#include <X11/extensions/Xinerama.h>
#else
#include <vulkan/vulkan.h>
#endif

// apt-get install libxinerama-dev

#ifdef KORE_OPENGL
void enumDisplayMonitors(kinc_display_t *displays, int& displayCounter) {
    ::Display* dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        log(Kore::Error, "Could not open display");
        exit(1);
        return;
    }

    int eventBase;
    int errorBase;

    if (XineramaQueryExtension(dpy, &eventBase, &errorBase)) {
        if (XineramaIsActive(dpy)) {
            int heads = 0;
            XineramaScreenInfo* queried = XineramaQueryScreens(dpy, &heads);

            for (int head = 0; head < heads; ++head) {
                ++displayCounter;
                XineramaScreenInfo& info = queried[head];
                // log(Info, "Head %i: %ix%i @%i;%i", head + 1, info.width, info.height, info.x_org, info.y_org);
	            kinc_display_t& di = displays[displayCounter];
                di.available = true;
                di.x = info.x_org;
                di.y = info.y_org;
                di.width = info.width;
                di.height = info.height;

                // TODO (DK)
                //      -is this always correct? if i switch screens on deb8/jessie with gnome it works ok
                //      -what about other *nix or window managers?
                di.primary = displayCounter == 0;

                // TODO (DK)
                //      -this doesn't work yet, whatever is configured as primary is the first screen returned,
                //       not what shows up in the config tool as [1], [2], ...
                //      -and info.screen_number just seems to be useless (0 for first returned, 1 for next, ...)
                di.number = info.screen_number + 1;
            }

            XFree(queried);
        }
        else {
            log(Kore::Warning, "Xinerama is not active");
        }
    }
    else {
        log(Kore::Warning, "Xinerama extension is not installed");
	    kinc_display_t &di = displays[0];
        di.available = true;
        di.x = 0;
        di.y = 0;
        di.width = 1920;
        di.height = 1080;
        di.primary = true;
        di.number = 0;
    }
}
#endif
//#endif // #if defined(KORE_LINUX_DISPLAY_XINERAMA)
