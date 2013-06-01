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
		int screenWidth();
		int screenHeight();
		void setTitle(const char* title);
		void showWindow();
		void swapBuffers();

#ifdef SYS_WINDOWS
		typedef __int64 ticks;
#elif defined SYS_WINDOWSRT
		typedef __int64 ticks;
#elif defined SYS_LINUX
		typedef unsigned long long ticks;
#elif defined SYS_OSX
		typedef unsigned long long ticks;
#elif defined SYS_IOS
		typedef unsigned long long ticks;
#elif defined SYS_ANDROID
		typedef unsigned long long ticks;
#else
		typedef uint64_t ticks;
#endif

		ticks getFrequency();
		ticks getTimestamp();
	}
}
