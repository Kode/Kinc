#pragma once

namespace Kore {
	struct RendererOptions {
		int textureFormat;
		int depthBufferBits;
		int stencilBufferBits;
		int antialiasing;

		RendererOptions() {
			textureFormat = 0;
			depthBufferBits = 16;
			stencilBufferBits = 8;
			antialiasing = 0;
		}
	};

	enum WindowMode {
		WindowModeWindow = 0,
		WindowModeBorderless = 1,
		WindowModeFullscreen = 2,
	};

    struct WindowOptions {
        const char * title;
        int width;
        int height;
        int x;
        int y;
		int targetDisplay;
		
		bool resizable;
		bool maximizable;
		bool minimizable;
		
		WindowMode mode;
		bool showWindow;
		RendererOptions rendererOptions;

		WindowOptions() {
			showWindow = true;
			title = "KoreWindow";
			targetDisplay = -1;
			mode = WindowModeWindow;
			rendererOptions.antialiasing = 0;
			x = y = -1;
			width = 800;
			height = 600;
			showWindow = true;
			
			resizable = false;
			maximizable = false;
			minimizable = true;
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
