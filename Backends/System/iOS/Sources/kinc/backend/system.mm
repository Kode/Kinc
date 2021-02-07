#include "pch.h"

#import "KoreAppDelegate.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/input/gamepad.h>
#include <kinc/input/keyboard.h>
#include <kinc/system.h>
#include <kinc/window.h>

#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>

extern "C" {
    bool withAutoreleasepool(bool (*f)()) {
        @autoreleasepool {
            return f();
        }
    }
}

static bool keyboardshown = false;

const char* iphonegetresourcepath() {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:1];
}

bool kinc_internal_handle_messages(void) {
	SInt32 result;
	do {
		result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
	} while (result == kCFRunLoopRunHandledSource);
	return true;
}

void kinc_set_keep_screen_on(bool on) {}

void showKeyboard();
void hideKeyboard();

void kinc_keyboard_show() {
	keyboardshown = true;
	::showKeyboard();
}

void kinc_keyboard_hide() {
	keyboardshown = false;
	::hideKeyboard();
}

bool kinc_keyboard_active() {
	return keyboardshown;
}

void loadURL(const char* url);

void kinc_load_url(const char* url) {
	::loadURL(url);
}

// On iOS you can't set the length of the vibration.
void kinc_vibrate(int ms) {
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
};

static char language[3];

const char* kinc_language() {
	NSString* nsstr = [[NSLocale preferredLanguages] objectAtIndex:0];
	const char* lang = [nsstr UTF8String];
	language[0] = lang[0];
	language[1] = lang[1];
	language[2] = 0;
	return language;
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

void kinc_internal_shutdown() {

}

int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_internal_init_window_options(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_internal_init_framebuffer_options(&defaultFrame);
		frame = &defaultFrame;
	}
	kinc_g4_init(0, frame->depth_bits, frame->stencil_bits, true);
	return 0;
}

void endGL();

void swapBuffersiOS() {
	endGL();
}

namespace {
	char sysid[512];
}

const char* kinc_system_id() {
	const char* name = [[[UIDevice currentDevice] name] UTF8String];
	const char* vendorId = [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
	strcpy(sysid, name);
	strcat(sysid, "-");
	strcat(sysid, vendorId);
	return sysid;
}

namespace {
	const char* getSavePath() {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString* resolvedPath = [paths objectAtIndex:0];
		NSString* appName = [NSString stringWithUTF8String:kinc_application_name()];
		resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

		NSFileManager* fileMgr = [[NSFileManager alloc] init];

		NSError* error;
		[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

		resolvedPath = [resolvedPath stringByAppendingString:@"/"];
		return [resolvedPath cStringUsingEncoding:1];
	}
}

const char* kinc_internal_save_path() {
	return getSavePath();
}

namespace {
	const char* videoFormats[] = {"mp4", nullptr};
}

const char** kinc_video_formats() {
	return ::videoFormats;
}

#include <mach/mach_time.h>

double kinc_frequency() {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

kinc_ticks_t kinc_timestamp() {
	kinc_ticks_t time = mach_absolute_time();
	return time;
}

void kinc_login() {

}

void kinc_unlock_achievement(int id) {
	
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "nobody";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "none";
}

bool kinc_gamepad_connected(int num) {
	return true;
}

int main(int argc, char* argv[]) {
	int retVal = 0;
	@autoreleasepool {
		[KoreAppDelegate description]; // otherwise removed by the linker
		retVal = UIApplicationMain(argc, argv, nil, @"KoreAppDelegate");
	}
	return retVal;
}
