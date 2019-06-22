#pragma once

typedef unsigned long XID;
struct __GLXcontextRec;

namespace Kore {
    struct WindowData {
        XID handle;
#ifdef KORE_OPENGL
        __GLXcontextRec* context;
#endif
        int width, height, mode;
    };
}
