#pragma once

typedef unsigned long XID;
struct __GLXcontextRec;

struct KincWindowData {
	XID handle;
#ifdef KORE_OPENGL
	struct __GLXcontextRec *context;
#endif
	int width, height, mode;
	void (*resizeCallback)(int x, int y, void *data);
	void *resizeCallbackData;
};
