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
#elif defined SYS_LINUX
		typedef unsigned long long ticks;
#elif defined SYS_OSX
		typedef unsigned long long ticks;
#elif defined SYS_IOS
		typedef unsigned long long ticks;
		#else
typedef uint64_t ticks;
		#endif

		ticks getFrequency();
		ticks getTimestamp();
	}
}

/*
#if 0
namespace {
	static Kt::uint currentframe = 0;
}

void nextframe() {
	++currentframe;
}

double Kt::Scheduler::getFrequency() {
	return 60;
}

Kt::Scheduler::ticks Kt::Scheduler::getTimestamp() {
	return currentframe * getFrequency() / 60;
}
#endif

#ifdef SYS_ANDROID

#include <sys/time.h>
#include <time.h>

double Kt::Scheduler::getFrequency() {
	return 1000000;
}

Kt::Scheduler::ticks Kt::Scheduler::getTimestamp() {
	timeval now;
	gettimeofday(&now, NULL);
	return static_cast<ticks>(now.tv_sec) * 1000000 + static_cast<ticks>(now.tv_usec);
}

#endif
*/
