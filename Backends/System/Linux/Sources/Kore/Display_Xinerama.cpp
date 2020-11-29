#include "pch.h"

#include <kinc/display.h>

#include <stdlib.h>

#include <Kore/Display.h>
#include <kinc/log.h>

#include <X11/X.h>
#include <X11/extensions/Xinerama.h>

typedef struct {
    bool available;
    int x;
    int y;
    int width;
    int height;
    bool primary;
    int number;
} kinc_display_t;

void enumDisplayMonitors(kinc_display_t *displays, int& displayCounter) {
    ::Display* dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        kinc_log(KINC_LOG_LEVEL_ERROR, "Could not open display");
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
            kinc_log(KINC_LOG_LEVEL_WARNING, "Xinerama is not active");
        }
    }
    else {
        kinc_log(KINC_LOG_LEVEL_WARNING, "Xinerama extension is not installed");
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
