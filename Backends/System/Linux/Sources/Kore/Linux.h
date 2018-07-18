#pragma once

struct _XDisplay;
typedef unsigned long XID;

namespace Kore {
    namespace Linux {
        extern _XDisplay* display;
        void fullscreen(XID window, bool value);
    }
}
