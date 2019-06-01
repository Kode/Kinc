#include "pch.h"

#import "BasicOpenGLView.h"

#import <Cocoa/Cocoa.h>

#include <kinc/graphics4/graphics.h>
#include <Kore/Input/HIDManager.h>
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
-(void)windowWillClose:(NSNotification*)notification;
-(void)windowDidResize:(NSNotification*)notification;
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
	[view begin];
}

void endGL() {
	[view end];
}

void newRenderPass(kinc_g5_render_target_t *renderTarget, bool wait) {
	[view newRenderPass: renderTarget wait: wait];
}

#endif

bool Kinc_Internal_HandleMessages() {
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
	[windows[windowId].view switchBuffers];
#endif
}

int createWindow(Kinc_WindowOptions* options) {
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

int Kinc_CountWindows() {
	return windowCounter;
}

void Kinc_Window_ChangeWindowMode(int window_index, Kinc_WindowMode mode) {
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

int kinc_init(const char* name, int width, int height, Kinc_WindowOptions* win, Kinc_FramebufferOptions* frame) {
	//System::_init(name, width, height, &win, &frame);
	Kinc_WindowOptions defaultWindowOptions;
	if (win == NULL) {
		Kinc_Internal_InitWindowOptions(&defaultWindowOptions);
		win = &defaultWindowOptions;
	}
	
	Kinc_FramebufferOptions defaultFramebufferOptions;
	if (frame == NULL) {
		Kinc_Internal_InitFramebufferOptions(&defaultFramebufferOptions);
		frame = &defaultFramebufferOptions;
	}
	
	int windowId = createWindow(win);
	kinc_g4_init(windowId, frame->depth_bits, frame->stencil_bits, true);
	return 0;
}

int Kinc_WindowWidth(int window_index) {
	NSWindow* window = windows[window_index].handle;
	return [[window contentView] frame].size.width;
}

int Kinc_WindowHeight(int window_index) {
	NSWindow* window = windows[window_index].handle;
	return [[window contentView] frame].size.height;
}

void Kinc_Internal_Shutdown() {
	
}

namespace {
	const char* savePath = nullptr;

	void getSavePath() {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString* resolvedPath = [paths objectAtIndex:0];
		NSString* appName = [NSString stringWithUTF8String: kinc_application_name()];
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

const char* Kinc_Internal_SavePath() {
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

#ifdef KOREC
extern "C"
#endif
int kore(int, char**);

@implementation MyApplication

- (void)run {
	@autoreleasepool {
		[self finishLaunching];
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

		hidManager = new Kore::HIDManager();
		addMenubar();

		// try {
		kore(argc, argv);
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

-(void)windowWillClose:(NSNotification*)notification {
	kinc_stop();
}

-(void)windowDidResize:(NSNotification*)notification {
	NSWindow* window = [notification object];
	NSSize size = [[window contentView]frame].size;
	[view resize:size];
	if (windows[0].resizeCallback != nullptr) {
		windows[0].resizeCallback(size.width, size.height, windows[0].resizeCallbackData);
	}
}

@end
