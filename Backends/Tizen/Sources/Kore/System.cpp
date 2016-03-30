#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
#include <Kore/Audio/Audio.h>
#include <Kore/IO/FileReader.h>
#include <Kore/IO/miniz.h>
#include <Kore/Input/Mouse.h>
#include <gl2.h>
#include <cstring>

void* Kore::System::createWindow() {
	return nullptr;
}

void Kore::System::swapBuffers() {

}

void Kore::System::destroyWindow() {

}

void Kore::System::changeResolution(int, int, bool) {

}

void Kore::System::showKeyboard() {

}

void Kore::System::hideKeyboard() {

}

void Kore::System::loadURL(const char* url) {
    
}

void Kore::System::setTitle(const char*) {

}

void Kore::System::setKeepScreenOn( bool on ) {
    
}

void Kore::System::showWindow() {
    
}

extern int kore(int argc, char** argv);

namespace {
	void init() {
		kore(0, nullptr);
	}
}

namespace {
	using namespace Kore;

	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}
}

bool Kore::System::handleMessages() {
	return true;
}

int Kore::System::screenWidth() {
	return Application::the()->width();
}

int Kore::System::screenHeight() {
	return Application::the()->height();
}

#include <sys/time.h>
#include <time.h>

double Kore::System::frequency() {
	return 1000000.0;
}

Kore::System::ticks Kore::System::timestamp() {
	timeval now;
	gettimeofday(&now, NULL);
	return static_cast<ticks>(now.tv_sec) * 1000000 + static_cast<ticks>(now.tv_usec);
}
