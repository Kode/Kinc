//#define KORE_LINUX_SYSTEM_SCREENS_XINERAMA 1

//#if defined(KORE_LINUX_SYSTEM_SCREENS_XINERAMA)

#include "pch.h"
#include "System_Screens.h"
#include <Kore/Log.h>

#include <X11/extensions/Xinerama.h>

namespace Kore { namespace System { namespace Monitor {
    void fatalError(const char* message);

void enumDisplayMonitors( KoreScreen screens[], int & screenCounter ) {
    Display * dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        fatalError("Could not open display");
    }

    int eventBase;
    int errorBase;

    if (XineramaQueryExtension(dpy, &eventBase, &errorBase)) {
        if (XineramaIsActive(dpy)) {
            int heads = 0;
            XineramaScreenInfo * queried = XineramaQueryScreens(dpy, &heads);

            for (int head = 0; head < heads; ++head) {
                ++screenCounter;
                XineramaScreenInfo & info = queried[head];
                log(Info, "Head %i: %ix%i @%i;%i", head + 1, info.width, info.height, info.x_org, info.y_org);
                KoreScreen & screen = screens[screenCounter];
                screen.isAvailable = true;
                screen.x = info.x_org;
                screen.y = info.y_org;
                screen.width = info.width;
                screen.height = info.height;

                // TODO (DK)
                //      -is this always correct? if i switch screens on deb8/jessie with gnome it works ok
                //      -what about other *nix or window managers?
                screen.isPrimary = screenCounter == 0;

                // TODO (DK)
                //      -this doesn't work yet, whatever is configured as primary is the first screen returned,
                //      not what shows up in the config tool as [1], [2], ...
                //      -and info.screen_number just seems to be useless (0 for first returned, 1 for next, ...)
                screen.number = info.screen_number + 1;
            }

            XFree(queried);
        } else {
            log(Warning, "Xinerama is not active");
        }
    } else {
        log(Warning, "Xinerama extension is not installed");
    }
}

}}}

//#endif // #if defined(KORE_LINUX_SYSTEM_SCREENS_XINERAMA)
