#pragma once

namespace Kore {
	class Display;

	struct FramebufferOptions {
		int frequency;
		bool verticalSync;
		int colorBufferBits;
		int depthBufferBits;
		int stencilBufferBits;
		int samplesPerPixel;

		FramebufferOptions() : frequency(60), verticalSync(true), colorBufferBits(32), depthBufferBits(16), stencilBufferBits(8), samplesPerPixel(1) {}
	};

	enum WindowMode {
		WindowModeWindow = 0,
		WindowModeFullscreen = 1,
		WindowModeExclusiveFullscreen = 2, // Only relevant for Windows
	};

	const int WindowFeatureResizable = 1;
	const int WindowFeatureMinimizable = 2;
	const int WindowFeatureMaximizable = 4;
	const int WindowFeatureBorderless = 8;
	const int WindowFeatureOnTop = 16;

	struct WindowOptions {
		const char *title;

		int x;
		int y;
		int width;
		int height;
		int displayIndex;

		bool visible;
		int windowFeatures;
		WindowMode mode;

		WindowOptions()
		    : title("Kore"), x(-1), y(-1), width(800), height(600), displayIndex(-1), visible(true),
		      windowFeatures(WindowFeatureResizable | WindowFeatureMinimizable | WindowFeatureMaximizable), mode(WindowModeWindow) {}
	};

	class Window {
	public:
		static Window *create(WindowOptions *win = nullptr, FramebufferOptions *frame = nullptr);
		static void destroy(Window *window);
		static Window *get(int index);
		static int count();
		void resize(int width, int height);
		void move(int x, int y);
		void changeWindowMode(WindowMode mode);
		void changeWindowFeatures(int features);
		void changeFramebuffer(FramebufferOptions *frame);
		int x();
		int y();
		int width();
		int height();
		Display *display();
		WindowMode mode();
		void show();
		void hide();
		void setTitle(const char *title);
		void setResizeCallback(void (*callback)(int x, int y, void *data), void *data = nullptr);
		void setPpiChangedCallback(void (*callback)(int ppi, void *data), void *data = nullptr);
		bool vSynced();

		int _index;
		Window();
	};
}
