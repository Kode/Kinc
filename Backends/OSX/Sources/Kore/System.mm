#include "pch.h"

#import "BasicOpenGLView.h"

#import <Cocoa/Cocoa.h>

#include <Kore/Graphics/Graphics.h>
#include <Kore/Input/HIDManager.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Log.h>
#include <Kore/System.h>

using namespace Kore;

extern const char* macgetresourcepath();

const char* macgetresourcepath() {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:1];
}

@interface MyApplication : NSApplication {
	bool shouldKeepRunning;
}

- (void)run;
- (void)terminate:(id)sender;

@end

@interface MyAppDelegate : NSObject <NSWindowDelegate> {
}

- (void)windowWillClose:(NSNotification*)notification;

@end

namespace {
	NSApplication* myapp;
	//	NSWindow* window;
	BasicOpenGLView* view;
	MyAppDelegate* delegate;
	HIDManager* hidManager;

	struct KoreWindow : public KoreWindowBase {
		NSWindow* handle;
		BasicOpenGLView* view;

		KoreWindow(NSWindow* handle, BasicOpenGLView* view, int x, int y, int width, int height)
		    : KoreWindowBase(x, y, width, height), handle(handle), view(view) {
			::view = view;
		}
	};

	KoreWindow* windows[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	int windowCounter = -1;
}

#ifdef SYS_METAL

id getMetalDevice() {
	return [view metalDevice];
}

id getMetalLibrary() {
	return [view metalLibrary];
}

id getMetalEncoder() {
	return [view metalEncoder];
}

void beginGL() {
	[view begin];
}

void endGL() {
	[view end];
}

#endif

bool System::handleMessages() {
	NSEvent* event =
	    [myapp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]; // distantPast: non-blocking
	if (event != nil) {
		[myapp sendEvent:event];
		[myapp updateWindows];
	}
	return true;
}

void System::swapBuffers(int windowId) {
#ifdef SYS_METAL
	endGL();
#else
	[windows[windowId]->view switchBuffers];
#endif
}

int Kore::System::windowCount() {
	return windowCounter + 1;
}

int createWindow(const char* title, int x, int y, int width, int height, WindowMode windowMode, int targetDisplay) {
	BasicOpenGLView* view = [[BasicOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
	NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
	                                               styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
	                                                 backing:NSBackingStoreBuffered
	                                                   defer:TRUE];
	delegate = [MyAppDelegate alloc];
	[window setDelegate:delegate];
	[window setTitle:[NSString stringWithCString:title encoding:1]];
	[window setAcceptsMouseMovedEvents:YES];
	[[window contentView] addSubview:view];
	[window center];
	[window makeKeyAndOrderFront:nil];

	++windowCounter;
	windows[windowCounter] = new KoreWindow(window, view, x, y, width, height);
	Kore::System::makeCurrent(windowCounter);
	return windowCounter;
}

void System::destroyWindow(int windowId) {}

int Kore::System::initWindow(Kore::WindowOptions options) {
	int id = createWindow(options.title, options.x, options.y, options.width, options.height, options.mode, options.targetDisplay);
	Graphics::init(id, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
	return id;
}

#ifndef SYS_METAL
void Graphics::makeCurrent(int contextId) {
	//[[windows[contextId]->view openGLContext] makeCurrentContext];
}
#endif

int Kore::System::windowWidth(int id) {
	return windows[id]->width;
}

int Kore::System::windowHeight(int id) {
	return windows[id]->height;
}

int System::desktopWidth() {
	NSArray* screenArray = [NSScreen screens];
	// unsigned screenCount = [screenArray count];
	NSScreen* screen = [screenArray objectAtIndex:0];
	NSRect screenRect = [screen visibleFrame];
	return screenRect.size.width;
}

int System::desktopHeight() {
	NSArray* screenArray = [NSScreen screens];
	// unsigned screenCount = [screenArray count];
	NSScreen* screen = [screenArray objectAtIndex:0];
	NSRect screenRect = [screen visibleFrame];
	return screenRect.size.height;
}

namespace {
	const char* savePath = nullptr;

	void getSavePath() {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString* resolvedPath = [paths objectAtIndex:0];
		NSString* appName = [NSString stringWithUTF8String:Kore::System::name()];
		resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

		NSFileManager* fileMgr = [[NSFileManager alloc] init];

		NSError* error;
		[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

		resolvedPath = [resolvedPath stringByAppendingString:@"/"];
		savePath = [resolvedPath cStringUsingEncoding:1];
	}

	int argc = 0;
	char** argv = nullptr;
}

const char* System::savePath() {
	if (::savePath == nullptr) getSavePath();
	return ::savePath;
}

int main(int argc, char** argv) {
	::argc = argc;
	::argv = argv;
	@autoreleasepool {
		myapp = [MyApplication sharedApplication];
		[myapp performSelectorOnMainThread:@selector(run) withObject:nil waitUntilDone:YES];
	}
	return 0;
}

#ifdef KOREC
extern "C"
#endif
    void
    kore(int, char**);

@implementation MyApplication

- (void)run {
	@autoreleasepool {
		[self finishLaunching];
		hidManager = new HIDManager();
		// try {
		kore(argc, argv);
		//}
		// catch (Kt::Exception& ex) {
		//	printf("Exception caught");
		//}
	}
}

- (void)terminate:(id)sender {
	Kore::System::stop();
}

@end

@implementation MyAppDelegate

- (void)windowWillClose:(NSNotification*)notification {
	Kore::System::stop();
}

@end
