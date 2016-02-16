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
        fatalError("could not open display");
    }

    int eventBase;
    int errorBase;

    if (XineramaQueryExtension(dpy, &eventBase, &errorBase)) {
        if (XineramaIsActive(dpy)) {
            int heads = 0;
            XineramaScreenInfo * screens = XineramaQueryScreens(dpy, &heads);

            for (int head = 0; head < heads; ++head) {
                XineramaScreenInfo & screen = screens[head];
                log(Info, "head %i: %ix%i @ %i; %i", head + 1, screen.width, screen.height, screen.x_org, screen.y_org);
            }

            XFree(screens);
        } else {
            log(Warning, "Xinerama not active");
        }
    } else {
        log(Warning, "Xinerama extension not installed");
    }
}

}}}

//#endif // #if defined(KORE_LINUX_SYSTEM_SCREENS_XINERAMA)
