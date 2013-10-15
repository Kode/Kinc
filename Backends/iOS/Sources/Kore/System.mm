#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
#include <Kore/Input/KeyEvent.h>
#import <UIKit/UIKit.h>
#import "KoreAppDelegate.h"

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyboardshown = false;
	int theScreenWidth;
	int theScreenHeight;
}

const char* iphonegetresourcepath() {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:1];
}

bool System::handleMessages() {
	SInt32 result;
	do {
		result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
	}
	while(result == kCFRunLoopRunHandledSource);
	return true;
}

vec2i System::mousePos() {
	return vec2i(mouseX, mouseY);
}

void* System::createWindow() {
	//Scheduler::addFrameTask(handleMessages, 1001);
	return nullptr;
}

void System::destroyWindow() {
	
}

void System::changeResolution(int width, int height, bool fullscreen) {

}

void System::showWindow() {
	
}

void System::setTitle(const char* title) {
	
}

bool System::showsKeyboard() {
	return keyboardshown;
}

void showKeyboard();
void hideKeyboard();

void System::showKeyboard() {
	keyboardshown = true;
	::showKeyboard();
}

void System::hideKeyboard() {
	keyboardshown = false;
    ::hideKeyboard();
}

void endGL();

void System::swapBuffers() {
	endGL();
}

int System::screenWidth() {
#ifdef ROTATE90
	return theScreenHeight;
#else
	return theScreenWidth;
#endif
}

int System::screenHeight() {
#ifdef ROTATE90
	return theScreenWidth;
#else
	return theScreenHeight;
#endif
}

#include <mach/mach_time.h>

double System::frequency() {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

System::ticks System::timestamp() {
	System::ticks time = mach_absolute_time();
	return time;
}

int main(int argc, char *argv[]) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	CGRect screenBounds = [[UIScreen mainScreen] bounds];
	CGFloat screenScale = [[UIScreen mainScreen] scale];
	//screenScale = 1.0f; // Temporary hack
	theScreenWidth = static_cast<int>(screenBounds.size.width * screenScale);
	theScreenHeight = static_cast<int>(screenBounds.size.height * screenScale);
	[KoreAppDelegate description]; //otherwise removed by the linker
	[UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationLandscapeRight;
	int retVal = UIApplicationMain(argc, argv, nil, @"KoreAppDelegate");
	
	[pool drain];
	
	return retVal;
}