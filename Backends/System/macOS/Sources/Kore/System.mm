#include "pch.h"

#import "BasicOpenGLView.h"

#import <Cocoa/Cocoa.h>

#include <kinc/graphics4/graphics.h>
#include <Kore/Input/HIDManager.h>
#include <Kore/System.h>
#include <kinc/input/keyboard.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "WindowData.h"

extern "C" {
	bool withAutoreleasepool(bool (*f)()) {
		@autoreleasepool {
			return f();
		}
	}
}

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
- (void)windowWillClose:(NSNotification*)notification;
- (void)windowDidResize:(NSNotification*)notification;
- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidResignMain:(NSNotification *)notification;
- (void)windowDidBecomeMain:(NSNotification *)notification;
@end

namespace {
	NSApplication* myapp;
	NSWindow* window;
	BasicOpenGLView* view;
	MyAppDelegate* delegate;
	Kore::HIDManager* hidManager;

	/*struct KoreWindow : public KoreWindowBase {
		NSWindow* handle;
		BasicOpenGLView* view;

		KoreWindow(NSWindow* handle, BasicOpenGLView* view, int x, int y, int width, int height)
		    : KoreWindowBase(x, y, width, height), handle(handle), view(view) {
			::view = view;
		}
	};*/

	Kore::WindowData windows[10] = {};
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
  if (window.visible) {
    [view begin];
  }
}

void endGL() {
  if (window.visible) {
    [view end];
  }
}

void newRenderPass(kinc_g5_render_target_t *renderTarget, bool wait) {
  if (window.visible) {
    [view newRenderPass: renderTarget wait: wait];
  }
}

#endif

bool kinc_internal_handle_messages() {
	NSEvent* event =
	    [myapp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]; // distantPast: non-blocking
	if (event != nil) {
		[myapp sendEvent:event];
		[myapp updateWindows];
	}
  
  // Sleep for a frame to limit the calls when the window is not visible.
  if (!window.visible) {
    [NSThread sleepForTimeInterval: 1.0 / 60];
  }
	return true;
}

void swapBuffersMac(int windowId) {
#ifdef KORE_METAL
	endGL();
#else
	[windows[windowId].view switchBuffers];
#endif
}

int createWindow(kinc_window_options_t *options) {
	int width = options->width;
	int height = options->height;
	int styleMask = NSTitledWindowMask | NSClosableWindowMask;
	if ((options->window_features & KINC_WINDOW_FEATURE_RESIZEABLE) || (options->window_features & KINC_WINDOW_FEATURE_MAXIMIZABLE)) {
		styleMask |= NSResizableWindowMask;
	}
	if (options->window_features & KINC_WINDOW_FEATURE_MINIMIZABLE) {
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
	
	windows[windowCounter].handle = window;
	windows[windowCounter].view = view;
	::window = window;
	::view = view;

	[window makeKeyAndOrderFront:nil];
	
	if (options->mode == KINC_WINDOW_MODE_FULLSCREEN || options->mode == KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
		[window toggleFullScreen:nil];
		windows[windowCounter].fullscreen = true;
	}
  
	return windowCounter++;
}

int kinc_count_windows() {
	return windowCounter;
}

void kinc_window_change_window_mode(int window_index, kinc_window_mode_t mode) {
	switch (mode) {
		case KINC_WINDOW_MODE_WINDOW:
			if (windows[window_index].fullscreen) {
				[window toggleFullScreen:nil];
				windows[window_index].fullscreen = false;
			}
			break;
		case KINC_WINDOW_MODE_FULLSCREEN:
		case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
			if (!windows[window_index].fullscreen) {
				[window toggleFullScreen:nil];
				windows[window_index].fullscreen = true;
			}
			break;
	}
	
}

int kinc_init(const char* name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	//System::_init(name, width, height, &win, &frame);
	kinc_window_options_t defaultWindowOptions;
	if (win == NULL) {
		kinc_internal_init_window_options(&defaultWindowOptions);
		win = &defaultWindowOptions;
	}
	
	kinc_framebuffer_options_t defaultFramebufferOptions;
	if (frame == NULL) {
		kinc_internal_init_framebuffer_options(&defaultFramebufferOptions);
		frame = &defaultFramebufferOptions;
	}
	
	int windowId = createWindow(win);
	kinc_g4_init(windowId, frame->depth_bits, frame->stencil_bits, true);
	return 0;
}

int kinc_window_width(int window_index) {
	NSWindow* window = windows[window_index].handle;
	return [[window contentView] frame].size.width;
}

int kinc_window_height(int window_index) {
	NSWindow* window = windows[window_index].handle;
	return [[window contentView] frame].size.height;
}

void kinc_load_url(const char* url) {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

static char language[3];

const char* kinc_language() {
	NSString* nsstr = [[NSLocale preferredLanguages] objectAtIndex:0];
	const char* lang = [nsstr UTF8String];
	language[0] = lang[0];
	language[1] = lang[1];
	language[2] = 0;
	return language;
}

void kinc_internal_shutdown() {
	
}

namespace {
	const char* getSavePath() {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString* resolvedPath = [paths objectAtIndex:0];
		NSString* appName = [NSString stringWithUTF8String: kinc_application_name()];
		resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

		NSFileManager* fileMgr = [[NSFileManager alloc] init];

		NSError* error;
		[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

		resolvedPath = [resolvedPath stringByAppendingString:@"/"];
		return [resolvedPath cStringUsingEncoding:NSUTF8StringEncoding];
	}

	int argc = 0;
	char** argv = nullptr;
}

const char* kinc_internal_save_path() {
	return getSavePath();
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

void addMenubar() {
	NSString* appName = [[NSProcessInfo processInfo] processName];

	NSMenu* appMenu = [NSMenu new];
	NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
	NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	[appMenu addItem:quitMenuItem];

	NSMenuItem* appMenuItem = [NSMenuItem new];
	[appMenuItem setSubmenu:appMenu];

	NSMenu* menubar = [NSMenu new];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
}

@implementation MyApplication

- (void)run {
	@autoreleasepool {
		[self finishLaunching];
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

		hidManager = new Kore::HIDManager();
		addMenubar();

		// try {
		kickstart(argc, argv);
		//}
		// catch (Kt::Exception& ex) {
		//	printf("Exception caught");
		//}
	}
}

- (void)terminate:(id)sender {
	kinc_stop();
}

@end

@implementation MyAppDelegate

- (void)windowWillClose:(NSNotification*)notification {
	kinc_stop();
}

- (void)windowDidResize:(NSNotification*)notification {
	NSWindow* window = [notification object];
	NSSize size = [[window contentView]frame].size;
	[view resize:size];
	if (windows[0].resizeCallback != nullptr) {
		windows[0].resizeCallback(size.width, size.height, windows[0].resizeCallbackData);
	}
}

- (void)windowWillMiniaturize:(NSNotification *)notification {
  Kore::System::_backgroundCallback();
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
  Kore::System::_foregroundCallback();
}

- (void)windowDidResignMain:(NSNotification *)notification {
  Kore::System::_pauseCallback();
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
  Kore::System::_resumeCallback();
}

@end
