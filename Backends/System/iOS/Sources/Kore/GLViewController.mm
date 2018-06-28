#include "pch.h"

#import "GLViewController.h"
#import "GLView.h"

#import <Foundation/Foundation.h>
#include <Kore/Math/Core.h>
#include <objc/runtime.h>

static GLView* glView;

void beginGL() {
	[glView begin];
}

void endGL() {
	[glView end];
}

#ifdef KORE_METAL
void newRenderPass(Kore::Graphics5::RenderTarget* renderTarget, bool wait) {
	[glView newRenderPass: renderTarget wait: wait];
}
#endif

void showKeyboard() {
	[glView showKeyboard];
}

void hideKeyboard() {
	[glView hideKeyboard];
}

id getMetalDevice() {
#ifdef KORE_METAL
	return [glView metalDevice];
#else
	return nil;
#endif
}

id getMetalLibrary() {
#ifdef KORE_METAL
	return [glView metalLibrary];
#else
	return nil;
#endif
}

id getMetalEncoder() {
#ifdef KORE_METAL
	return [glView metalEncoder];
#else
	return nil;
#endif
}

@implementation GLViewController

- (void)loadView {
	self.view = glView = [[GLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
}

@end
