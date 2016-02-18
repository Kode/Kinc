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

namespace { namespace callbacks {
	void (*callback)();
	void (*foregroundCallback)();
	void (*backgroundCallback)();
	void (*pauseCallback)();
	void (*resumeCallback)();
	void (*shutdownCallback)();
	void (*orientationCallback)(Kore::Orientation);
}}

void Kore::System::setCallback( void (*value)() ) {
    callbacks::callback = value;
}

void Kore::System::setForegroundCallback( void (*value)() ) {
    callbacks::foregroundCallback = value;
}

void Kore::System::setResumeCallback( void (*value)() ) {
    callbacks::resumeCallback = value;
}

void Kore::System::setPauseCallback( void (*value)() ) {
    callbacks::pauseCallback = value;
}

void Kore::System::setBackgroundCallback( void (*value)() ) {
    callbacks::backgroundCallback = value;
}

void Kore::System::setShutdownCallback( void (*value)() ) {
    callbacks::shutdownCallback = value;
}

void Kore::System::setOrientationCallback( void (*value)(Orientation) ) {
    callbacks::orientationCallback = value;
}

void Kore::System::callback() {
    if (callbacks::callback != nullptr) {
        callbacks::callback();
    }
}

void Kore::System::foregroundCallback() {
    if (callbacks::foregroundCallback != nullptr) {
        callbacks::foregroundCallback();
    }
}

void Kore::System::resumeCallback() {
    if (callbacks::resumeCallback != nullptr) {
        callbacks::resumeCallback();
    }
}

void Kore::System::pauseCallback() {
    if (callbacks::pauseCallback != nullptr) {
        callbacks::pauseCallback();
    }
}

void Kore::System::backgroundCallback() {
    if (callbacks::backgroundCallback != nullptr) {
        callbacks::backgroundCallback();
    }
}

void Kore::System::shutdownCallback() {
    if (callbacks::shutdownCallback != nullptr) {
        callbacks::shutdownCallback();
    }
}

void Kore::System::orientationCallback( Orientation orientation ) {
    if (callbacks::orientationCallback != nullptr) {
        callbacks::orientationCallback(orientation);
    }
}

namespace { namespace appstate {
	bool showWindowFlag = true;
	const char * name = "KoreApplication";
}}

void Kore::System::setShowWindowFlag( bool value ) {
    appstate::showWindowFlag = value;
}

bool Kore::System::hasShowWindowFlag() {
    return appstate::showWindowFlag;
}

void Kore::System::setName( const char * value ) {
    appstate::name = value; // TODO (DK) strcpy?
}

const char * Kore::System::name() {
	return appstate::name;
}
