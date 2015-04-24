#import "GLViewController.h"
#import "GLView.h"
#include "pch.h"
#import <Foundation/Foundation.h>
#include <Kore/Math/Core.h>

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

void* getMetalDevice() {
#ifdef SYS_METAL
	return (__bridge_retained void*)[glView metalDevice];
#else
	return nullptr;
#endif
}

void* getMetalLibrary() {
#ifdef SYS_METAL
	return (__bridge_retained void*)[glView metalLibrary];
#else
	return nullptr;
#endif
}

void* getMetalCommandQueue() {
#ifdef SYS_METAL
	return (__bridge_retained void*)[glView metalCommandQueue];
#else
	return nullptr;
#endif
}

@implementation GLViewController

- (void)loadView {
	self.view = glView = [[GLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
}

@end
