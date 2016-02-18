#pragma once

namespace Kore {
	struct RendererOptions {
		//int width;
		//int height;
		int textureFormat;
		int depthBufferBits;
		int stencilBufferBits;
		int antialiasing;
	};

    struct WindowOptions {
        const char * title;
        int width;
        int height;
        int x;
        int y;
        int targetDisplay;
        int mode;
		bool showWindow;
		RendererOptions rendererOptions;

		WindowOptions() {
			showWindow = true;
		}
    };

    struct KoreWindowBase {
		int x, y;
		int width, height;

		KoreWindowBase( int x, int y, int width, int height ) {
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
    };
}
