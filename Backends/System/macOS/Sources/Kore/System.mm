#include "pch.h"

#import "BasicOpenGLView.h"

#import <Cocoa/Cocoa.h>

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Input/HIDManager.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#ifdef KORE_METAL
namespace Kore {
	namespace Graphics5 {
		class RenderTarget;
	}
}
#endif

using namespace Kore;

extern const char* macgetresourcepath();

const char* macgetresourcepath() {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding];
}

@interface MyApplication : NSApplication {
	bool shouldKeepRunning;
}

- (void)run;
- (void)terminate:(id)sender;

@end

@interface MyAppDelegate : NSObject<NSWindowDelegate> {}
-(void)windowWillClose:(NSNotification*)notification;
-(void)windowDidResize:(NSNotification*)notification;
@end

namespace {
	NSApplication* myapp;
	NSWindow* window;
	BasicOpenGLView* view;
	MyAppDelegate* delegate;
	HIDManager* hidManager;

	/*struct KoreWindow : public KoreWindowBase {
		NSWindow* handle;
		BasicOpenGLView* view;

		KoreWindow(NSWindow* handle, BasicOpenGLView* view, int x, int y, int width, int height)
		    : KoreWindowBase(x, y, width, height), handle(handle), view(view) {
			::view = view;
		}
	};*/

	Kore::Window* windows[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	int windowCounter = 0;
}

#ifdef KORE_METAL

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

void newRenderPass(Kore::Graphics5::RenderTarget* renderTarget, bool wait) {
	[view newRenderPass: renderTarget wait: wait];
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

void swapBuffersMac(int windowId) {
#ifdef KORE_METAL
	endGL();
#else
	[windows[windowId]->view switchBuffers];
#endif
}

int createWindow(Kore::WindowOptions* options) {
	int width = options->width;
	int height = options->height;
	int styleMask = NSTitledWindowMask | NSClosableWindowMask;
	if ((options->windowFeatures & WindowFeatureResizable) || (options->windowFeatures & WindowFeatureMaximizable)) {
		styleMask |= NSResizableWindowMask;
	}
	if (options->windowFeatures & WindowFeatureMinimizable) {
		styleMask |= NSMiniaturizableWindowMask;
	}

	BasicOpenGLView* view = [[BasicOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
	[view registerForDraggedTypes:[NSArray arrayWithObjects:NSURLPboardType, nil]];
	NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height) styleMask:styleMask backing:NSBackingStoreBuffered defer:TRUE];
	delegate = [MyAppDelegate alloc];
	[window setDelegate:delegate];
	[window setTitle:[NSString stringWithCString:options->title encoding:NSUTF8StringEncoding]];
	[window setAcceptsMouseMovedEvents:YES];
	[[window contentView] addSubview:view];
	[window center];
	[window makeKeyAndOrderFront:nil];
	
	windows[windowCounter] = new Kore::Window; //new KoreWindow(window, view, options->x, options->y, width, height);
	windows[windowCounter]->_data.handle = window;
	windows[windowCounter]->_data.view = view;
	::window = window;
	::view = view;
	
	if (options->mode == WindowModeFullscreen || options->mode == WindowModeExclusiveFullscreen) {
		[window toggleFullScreen:nil];
		windows[windowCounter]->_data.fullscreen = true;
	}
	
	return windowCounter++;
}

Window* Window::get(int window) {
	return windows[window];
}

int Window::count() {
	return windowCounter;
}

void Window::changeWindowMode(WindowMode mode) {
	switch (mode) {
		case WindowModeWindow:
			if (_data.fullscreen) {
				[window toggleFullScreen:nil];
				_data.fullscreen = false;
			}
			break;
		case WindowModeFullscreen:
		case WindowModeExclusiveFullscreen:
			if (!_data.fullscreen) {
				[window toggleFullScreen:nil];
				_data.fullscreen = true;
			}
			break;
	}
	
}

Kore::Window* Kore::System::init(const char* title, int width, int height, Kore::WindowOptions* win, Kore::FramebufferOptions* frame) {
	WindowOptions defaultWin;
	FramebufferOptions defaultFrame;
	
	if (win == nullptr) {
		win = &defaultWin;
	}
	win->width = width;
	win->height = height;
	
	if (frame == nullptr) {
		frame = &defaultFrame;
	}
	
	int windowId = createWindow(win);
	Graphics4::init(windowId, frame->depthBufferBits, frame->stencilBufferBits);
	return Kore::Window::get(windowId);
}

#ifndef KORE_METAL
void Graphics4::makeCurrent(int contextId) {
	//[[windows[contextId]->view openGLContext] makeCurrentContext];
}
#endif

int Kore::Window::width() {
	NSWindow* window = _data.handle;
	return [[window contentView] frame].size.width;
}

int Kore::Window::height() {
	NSWindow* window = _data.handle;
	return [[window contentView] frame].size.height;
}

void Kore::System::_shutdown() {
	
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
		savePath = [resolvedPath cStringUsingEncoding:NSUTF8StringEncoding];
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

-(void)windowWillClose:(NSNotification*)notification {
	Kore::System::stop();
}

-(void)windowDidResize:(NSNotification*)notification {
	NSWindow* window = [notification object];
	NSSize size = [[window contentView]frame].size;
	[view resize:size];
	if (windows[0]->_data.resizeCallback != nullptr) {
		windows[0]->_data.resizeCallback(size.width, size.height, windows[0]->_data.resizeCallbackData);
	}
}

@end
