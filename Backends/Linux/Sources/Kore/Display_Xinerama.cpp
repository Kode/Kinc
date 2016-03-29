//#define KORE_LINUX_DISPLAY_XINERAMA 1

//#if defined(KORE_LINUX_DISPLAY_XINERAMA)

#include "pch.h"
#include "Display.h"
#include <Kore/Log.h>

#ifdef OPENGL
#include <X11/X.h>
#include <X11/extensions/Xinerama.h>
#else
#include <vulkan/vulkan.h>
#endif

//apt-get install libxinerama-dev

#ifdef OPENGL
namespace Kore { namespace Display {
void fatalError(const char* message);

void enumDisplayMonitors( DeviceInfo displays[], int & displayCounter ) {
    ::Display * dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        fatalError("Could not open display");
        return;
    }

    int eventBase;
    int errorBase;

    if (XineramaQueryExtension(dpy, &eventBase, &errorBase)) {
        if (XineramaIsActive(dpy)) {
            int heads = 0;
            XineramaScreenInfo * queried = XineramaQueryScreens(dpy, &heads);

            for (int head = 0; head < heads; ++head) {
                ++displayCounter;
                XineramaScreenInfo & info = queried[head];
                log(Info, "Head %i: %ix%i @%i;%i", head + 1, info.width, info.height, info.x_org, info.y_org);
                DeviceInfo & di = displays[displayCounter];
                di.isAvailable = true;
                di.x = info.x_org;
                di.y = info.y_org;
                di.width = info.width;
                di.height = info.height;

                // TODO (DK)
                //      -is this always correct? if i switch screens on deb8/jessie with gnome it works ok
                //      -what about other *nix or window managers?
                di.isPrimary = displayCounter == 0;

                // TODO (DK)
                //      -this doesn't work yet, whatever is configured as primary is the first screen returned,
                //       not what shows up in the config tool as [1], [2], ...
                //      -and info.screen_number just seems to be useless (0 for first returned, 1 for next, ...)
                di.number = info.screen_number + 1;
            }

            XFree(queried);
        } else {
            log(Warning, "Xinerama is not active");
        }
    } else {
        log(Warning, "Xinerama extension is not installed");
    }
}

}}
#endif
//#endif // #if defined(KORE_LINUX_DISPLAY_XINERAMA)
