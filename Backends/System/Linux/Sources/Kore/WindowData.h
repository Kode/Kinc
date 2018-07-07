#pragma once

typedef unsigned long XID;
struct __GLXcontextRec;

namespace Kore {
    struct WindowData {
        XID handle;
        __GLXcontextRec* context;
        int width, height;
        WindowData();
    };
}
