#import "KoreAppDelegate.h"
#import "GLView.h"
#include "pch.h"
#include <Kore/Application.h>
#include <wchar.h>

@implementation KoreAppDelegate

static UIWindow* window;
static GLView* glView;

void beginGL() {
	[glView begin];
}

void endGL() {
	[glView end];
}

void showKeyboard() {
	[glView showKeyboard];
}

void hideKeyboard() {
    [glView hideKeyboard];
}

int kore(int argc, char** argv);

- (void)mainLoop {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
	//try {
		kore(0, nullptr);
	//}
	//catch (Kt::Exception& ex) {
	//	printf("Exception\n");
	//	printf("%s", ex.what());
	//}
	
	[pool drain];
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
	//CGRect rect = [[UIScreen mainScreen] applicationFrame];
	
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];
	
	CGRect screenBounds = [[UIScreen mainScreen] bounds];
	
	glView = [[GLView alloc] initWithFrame:CGRectMake(0, 0, screenBounds.size.width, screenBounds.size.height)];
	
	[window addSubview:glView];
	[window makeKeyAndVisible];
	
	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];
	
    return YES;
}

- (void)applicationWillEnterForeground:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->foregroundCallback != nullptr) Kore::Application::the()->foregroundCallback();
}

- (void)applicationDidBecomeActive:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->resumeCallback != nullptr) Kore::Application::the()->resumeCallback();
}

- (void)applicationWillResignActive:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->pauseCallback != nullptr) Kore::Application::the()->pauseCallback();
}

- (void)applicationDidEnterBackground:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->backgroundCallback != nullptr) Kore::Application::the()->backgroundCallback();
}

- (void)applicationWillTerminate:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->shutdownCallback != nullptr) Kore::Application::the()->shutdownCallback();
}

- (void)dealloc {
    [window release];
    [glView release];
    [super dealloc];
}

@end
