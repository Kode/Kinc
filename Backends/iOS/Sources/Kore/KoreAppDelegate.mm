#import "KoreAppDelegate.h"
#import "GLView.h"
#import "GLViewController.h"
#include "pch.h"
#include <Kore/Application.h>
#include <Kore/Math/Core.h>
#include <wchar.h>

@implementation KoreAppDelegate

static UIWindow* window;

void loadURL(const char* url) {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

int kore(int argc, char** argv);

- (void)mainLoop {
	//NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
	//try {
	@autoreleasepool {
		kore(0, nullptr);
	}
	//}
	//catch (Kt::Exception& ex) {
	//	printf("Exception\n");
	//	printf("%s", ex.what());
	//}
	
	//[pool drain];
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
	//CGRect rect = [[UIScreen mainScreen] applicationFrame];
	CGRect screenBounds = [[UIScreen mainScreen] bounds];
	
	//window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height), Kore::max(screenBounds.size.width, screenBounds.size.height))];
	CGRect bounds = [[UIScreen mainScreen] bounds];
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];
	
	//glView = [[GLView alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height), Kore::max(screenBounds.size.width, screenBounds.size.height))];
	GLViewController* glViewController = [[GLViewController alloc] init];
	//glViewController.view = glView;
	//[glViewController ]
	//[window addSubview:glView];
	[window setRootViewController:glViewController];
	[window makeKeyAndVisible];
	
	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];
	
    return YES;
}

static Kore::Orientation convertOrientation(UIDeviceOrientation orientation) {
	switch (orientation) {
		case UIDeviceOrientationLandscapeLeft:
			return Kore::OrientationLandscapeRight;
		case UIDeviceOrientationLandscapeRight:
			return Kore::OrientationLandscapeLeft;
		case UIDeviceOrientationPortrait:
			return Kore::OrientationPortrait;
		case UIDeviceOrientationPortraitUpsideDown:
			return Kore::OrientationPortraitUpsideDown;
		default:
			return Kore::OrientationUnknown;
	}
}

static UIInterfaceOrientation convertAppleOrientation(UIDeviceOrientation orientation, UIInterfaceOrientation lastOrientation) {
    switch (orientation) {
		case UIDeviceOrientationLandscapeLeft:
			return UIInterfaceOrientationLandscapeRight;
		case UIDeviceOrientationLandscapeRight:
			return UIInterfaceOrientationLandscapeLeft;
		case UIDeviceOrientationPortrait:
			return UIInterfaceOrientationPortrait;
		case UIDeviceOrientationPortraitUpsideDown:
			return UIInterfaceOrientationPortraitUpsideDown;
		default:
			return lastOrientation;
	}
}

void KoreUpdateKeyboard();
/*
- (void)didRotate:(NSNotification*)notification {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->orientationCallback != nullptr) Kore::Application::the()->orientationCallback(convertOrientation([[UIDevice currentDevice] orientation]));
    [UIApplication sharedApplication].statusBarOrientation = convertAppleOrientation([[UIDevice currentDevice] orientation], [UIApplication sharedApplication].statusBarOrientation);
    KoreUpdateKeyboard();
}
*/
- (void)applicationWillEnterForeground:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->foregroundCallback != nullptr) Kore::Application::the()->foregroundCallback();
}

- (void)applicationDidBecomeActive:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->resumeCallback != nullptr) Kore::Application::the()->resumeCallback();
	//[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	//[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationWillResignActive:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->pauseCallback != nullptr) Kore::Application::the()->pauseCallback();
	//[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationDidEnterBackground:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->backgroundCallback != nullptr) Kore::Application::the()->backgroundCallback();
}

- (void)applicationWillTerminate:(UIApplication*)application {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->shutdownCallback != nullptr) Kore::Application::the()->shutdownCallback();
}

//- (void)dealloc {
//    [window release];
//    [glView release];
//    [super dealloc];
//}

@end
