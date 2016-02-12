#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Window.h>

namespace Kore {
	namespace System {
		int currentDevice();
		void setCurrentDevice(int id);
		int windowWidth(int id);
		int windowHeight(int id);

		int createWindow( int x, int y, int width, int height, int windowMode );
		void destroyWindow(int id);
		void* windowHandle(int windowId);
		void changeResolution(/*Window* window,*/ int width, int height, bool fullscreen);
		bool handleMessages();
		vec2i mousePos();
		void showKeyboard();
		void hideKeyboard();
		bool showsKeyboard();
		void loadURL(const char* title);
		int screenWidth();
		int screenHeight();
		int desktopWidth();
		int desktopHeight();
		const char* systemId();
		void setTitle(const char* title);
		const char* savePath();
		const char** videoFormats();
		void showWindow();
		void swapBuffers();

		typedef unsigned long long ticks;

		double frequency();
		ticks timestamp();
		double time();
	}
}
