#pragma once

struct _XDisplay;
typedef unsigned long XID;

extern struct _XDisplay *kinc_linux_display;
void kinc_linux_fullscreen(XID window, bool value);
