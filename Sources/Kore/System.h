#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Window.h>

namespace Kore {
	enum Orientation { OrientationLandscapeLeft, OrientationLandscapeRight, OrientationPortrait, OrientationPortraitUpsideDown, OrientationUnknown };

	// TODO (DK)
	//	remove windowing stuff from here and put into Kore::Windowing or something?
	//	better separation between windows + rendering contexts ie. Graphics?
	namespace System {
		void init(const char* name, int width, int height, int samplesPerPixel = 0);

		enum { MAXIMUM_WINDOW_COUNT = 10 };

		int currentDevice();
		// void setCurrentDevice(int id);

		int initWindow(WindowOptions options);
		void destroyWindow(int id);
		void* windowHandle(int windowId);
		int windowWidth(int id = 0);
		int windowHeight(int id = 0);
		int windowCount();

		int screenDpi();

		void changeResolution(int width, int height, bool fullscreen);
		bool handleMessages();
		vec2i mousePos();
		void showKeyboard();
		void hideKeyboard();
		bool showsKeyboard();
		void loadURL(const char* title);
		// int screenWidth(); // (DK) main window, better use windowWidth/Height( windowId )
		// int screenHeight(); // (DK) main window, better use windowWidth/Height( windowId )
		int desktopWidth();
		int desktopHeight();
		const char* systemId();
		void setTitle(const char* title);
		const char* savePath();
		const char** videoFormats();
		void showWindow();
		void swapBuffers(int contextId);
		void makeCurrent(int contextId);
		void clearCurrent();

		typedef unsigned long long ticks;

		double frequency();
		ticks timestamp();
		double time();

		// (DK) old application interface
		void setName(const char* name);
		const char* name();

		bool hasShowWindowFlag();           // TODO (DK) window specific?
		void setShowWindowFlag(bool value); // TODO (DK) window specific?

		int simpleSetup(int argc, char* argv[], int width, int height, int antialiasing = 0, WindowMode mode = WindowModeWindow, const char* title = "Kore",
		                bool showWindow = true);
		void setup();
		void start();
		bool frame();
		void stop();
		void _shutdown();
		bool isFullscreen();

		void setCallback(void (*value)());
		void setForegroundCallback(void (*value)());
		void setResumeCallback(void (*value)());
		void setPauseCallback(void (*value)());
		void setBackgroundCallback(void (*value)());
		void setShutdownCallback(void (*value)());
		void setOrientationCallback(void (*value)(Orientation));
		void setDropFilesCallback(void (*value)(wchar_t*));
		void setCutCallback(char* (*value)());
		void setCopyCallback(char* (*value)());
		void setPasteCallback(void (*value)(char*));
		void setKeepScreenOn(bool on);

		void callback();
		void foregroundCallback();
		void resumeCallback();
		void pauseCallback();
		void backgroundCallback();
		void shutdownCallback();
		void orientationCallback(Orientation);
		void dropFilesCallback(wchar_t*);
		char* cutCallback();
		char* copyCallback();
		void pasteCallback(char*);
	}
}
