#import "GLView.h"
#import "GLViewController.h"
#import "KoreAppDelegate.h"
#import <AVFAudio/AVFAudio.h>

#include <kinc/system.h>
#include <wchar.h>

@implementation KoreAppDelegate

static UIWindow *window;
static GLViewController *glViewController;

void loadURL(const char *url) {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]] options:@{} completionHandler:nil];
}

- (void)mainLoop {
	@autoreleasepool {
		kickstart(0, NULL);
	}
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	AVAudioSession *sessionInstance = [AVAudioSession sharedInstance];
	NSError *error;

	// set the session category
	NSString *category = AVAudioSessionCategoryAmbient;
	bool success = [sessionInstance setCategory:category error:&error];
	if (!success)
		NSLog(@"Error setting AVAudioSession category! %@\n", [error localizedDescription]);
	// CGRect rect = [[UIScreen mainScreen] applicationFrame];
	// CGRect screenBounds = [[UIScreen mainScreen] bounds];

	// window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height),
	// Kore::max(screenBounds.size.width, screenBounds.size.height))];
	// CGRect bounds = [[UIScreen mainScreen] bounds];
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];

	// glView = [[GLView alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height), Kore::max(screenBounds.size.width,
	// screenBounds.size.height))];
	glViewController = [[GLViewController alloc] init];
#ifndef KINC_TVOS
	glViewController.view.multipleTouchEnabled = YES;
#endif
	// glViewController.view = glView;
	//[glViewController ]
	//[window addSubview:glView];
	[window setRootViewController:glViewController];
	[window makeKeyAndVisible];

	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];

	return YES;
}

void KoreUpdateKeyboard(void);

- (void)applicationWillEnterForeground:(UIApplication *)application {
	[glViewController setVisible:YES];
	kinc_internal_foreground_callback();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	kinc_internal_resume_callback();
	//[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	//[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	kinc_internal_pause_callback();
	//[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	[glViewController setVisible:NO];
	kinc_internal_background_callback();
}

- (void)applicationWillTerminate:(UIApplication *)application {
	kinc_internal_shutdown_callback();
}

//- (void)dealloc {
//    [window release];
//    [glView release];
//    [super dealloc];
//}

@end
