#import "KoreAppDelegate.h"
#import "GLView.h"
#include "pch.h"
#include <wchar.h>

@implementation KoreAppDelegate

static UIWindow* window;
static GLView* glView;

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
	//CGRect rect = [[UIScreen mainScreen] applicationFrame];
	
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];
	
	CGRect screenBounds = [[UIScreen mainScreen] bounds];
	CGFloat screenScale = [[UIScreen mainScreen] scale];
	
	glView = [[GLView alloc] initWithFrame:CGRectMake(0, 0, screenBounds.size.width * screenScale, screenBounds.size.height * screenScale)];

	[window addSubview:glView];
	[window makeKeyAndVisible];
	
	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];
	
    return YES;
}

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

- (void)applicationWillResignActive:(UIApplication *)application {
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    
}

- (void)applicationWillTerminate:(UIApplication *)application {
    
}

- (void)dealloc {
    [window release];
    [glView release];
    [super dealloc];
}

@end