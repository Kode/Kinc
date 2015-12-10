#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
#import <UIKit/UIKit.h>
#import "KoreAppDelegate.h"

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyboardshown = false;
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

void loadURL(const char* url);

void System::loadURL(const char* url) {
	::loadURL(url);
}

// called on rotation event
void KoreUpdateKeyboard() {
    if (keyboardshown) {
        ::hideKeyboard();
        ::showKeyboard();
    }
    else {
        ::hideKeyboard();
    }
}

void endGL();

void System::swapBuffers() {
	endGL();
}

namespace {
	char sysid[512];
}

const char* System::systemId() {
	const char* name = [[[UIDevice currentDevice] name] UTF8String];
	const char* vendorId = [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
	strcpy(sysid, name);
	strcat(sysid, "-");
	strcat(sysid, vendorId);
	return sysid;
}

namespace {
	const char* savePath = nullptr;
	
	void getSavePath() {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString* resolvedPath = [paths objectAtIndex:0];
		NSString* appName = [NSString stringWithUTF8String:Application::the()->name()];
		resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];
		
		NSFileManager* fileMgr = [[NSFileManager alloc] init];
		
		NSError *error;
		[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];
		
		resolvedPath = [resolvedPath stringByAppendingString:@"/"];
		savePath = [resolvedPath cStringUsingEncoding:1];
	}
}

const char* System::savePath() {
	if (::savePath == nullptr) getSavePath();
	return ::savePath;
}

namespace {
	const char* videoFormats[] = { "mp4", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
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
	int retVal = 0;
	@autoreleasepool {
		[KoreAppDelegate description]; //otherwise removed by the linker
		retVal = UIApplicationMain(argc, argv, nil, @"KoreAppDelegate");
	}
	return retVal;
}