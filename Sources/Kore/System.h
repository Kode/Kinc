#pragma once

#include <Kore/Math/Vector.h>

namespace Kore {
	namespace System {
		void* createWindow();
		void destroyWindow();
		void* windowHandle();
		void changeResolution(int width, int height, bool fullscreen);
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
