#include "pch.h"
#include "System.h"

#ifndef SYS_HTML5
#ifndef SYS_ANDROID
double Kore::System::time() {
	return timestamp() / frequency();
}
#endif
#endif

#if !defined(SYS_WINDOWS) && !defined(SYS_OSX) && !defined(SYS_LINUX) && !defined(SYS_HTML5)

int Kore::System::desktopWidth() {
	return screenWidth();
}

int Kore::System::desktopHeight() {
	return screenHeight();
}

#endif // !ined(SYS_WINDOWS) && !defined(SYS_OSX) && !defined(SYS_LINUX) && !defined(SYS_HTML5)
