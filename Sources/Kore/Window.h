#pragma once

namespace Kore {
	struct RendererOptions {
		int width;
		int height;
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
		RendererOptions rendererOptions;
    };

    struct Window {
		int id;
		int width;
		int height;

		Window( int id, int width, int height ) {
			this->id = id;
			this->width = width;
			this->height = height;
		}
    };
}
